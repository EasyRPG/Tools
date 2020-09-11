/*
 * Copyright (c) 2020 LcfTrans authors
 * This file is released under the MIT License
 * http://opensource.org/licenses/MIT
 */

#include "translation.h"
#include "utils.h"

#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <fstream>
#include <lcf/context.h>
#include <lcf/data.h>
#include <lcf/rpg/eventcommand.h>
#include <lcf/ldb/reader.h>
#include <lcf/lmu/reader.h>
#include <lcf/lmt/reader.h>
#include <lcf/rpg/database.h>

using Cmd = lcf::rpg::EventCommand::Code;
constexpr int lines_per_message = 4;

void Translation::write(std::ostream& out) {
	writeHeader(out);
	out << std::endl;

	writeEntries(out);
}

void Translation::writeHeader(std::ostream& out) const {
	out << "msgid \"\"" << std::endl;
	out << "msgstr \"\"" << std::endl;
	out << "\"Project-Id-Version: GAME_NAME 1.0\\n\"" << std::endl;
	out << "\"Language-Team: YOUR NAME <mail@your.address>\\n\"" << std::endl;
	out << "\"Language: \\n\"" << std::endl;
	out << "\"MIME-Version: 1.0\\n\"" << std::endl;
	out << "\"Content-Type: text/plain; charset=UTF-8\\n\"" << std::endl;
	out << "\"Content-Transfer-Encoding: 8bit\\n\"" << std::endl;
	out << "\"X-CreatedBy: LcfTrans\"" << std::endl;
}

void Translation::writeEntries(std::ostream& out) {
	std::map<std::string, std::vector<Entry*>> items;
	std::vector<std::string> order;
	std::vector<Entry*> item;
	std::string old_context = "\1";

	for (Entry& e : entries) {
		std::string key = e.context + "\1" + e.original;

		items[key].push_back(&e);

		if (old_context != key && items[key].size() == 1) {
			old_context = key;
			order.push_back(key);
		}
	}

	for (std::string& s : order) {
		for (Entry* e : items[s]) {
			if (!e->location.empty()) {
				out << "#: " << e->location << "\n";
			}

			if (!e->info.empty()) {
				std::stringstream ss(e->info);

				std::string info;
				while (std::getline(ss, info, '\n')) {
					out << "#. " << info << "\n";
				}
			}
		}
		items[s][0]->write(out);

		out << "\n";
	}
}

bool Translation::addEntry(const Entry& entry) {
	if (entry.original.empty()) {
		return false;
	}
	entries.push_back(entry);
	return true;
}

const std::vector<Entry>& Translation::getEntries() const {
	return entries;
}

template<typename T, typename U, typename F>
static void parseEvents(Translation& t, lcf::StringView parent_name, U& root, const F& make_info) {
	std::vector<std::string> lines;
	std::vector<std::string> info;
	int prev_evt_id = 0;
	int prev_line = 0;
	int prev_indent = 0;

	auto add_evt_entry = [&]() {
		if (lines.empty()) {
			info.clear();
			return;
		}

		Entry e;
		e.original = Utils::Join(lines);
		e.info = Utils::Join(info);
		t.addEntry(e);
		lines.clear();
		info.clear();
	};

	// Read common events
	lcf::rpg::ForEachString(root, nullptr, [&](lcf::Context<lcf::DBString>& ctx) {
		if (!ctx.parent || !ctx.parent->parent) {
			return;
		}

		if (ctx.parent->parent->name != parent_name || ctx.name != "string") {
			// Only care about the text parameter of common events
			return;
		}

		const auto &evt = *reinterpret_cast<lcf::rpg::EventCommand*>(ctx.obj);

		int evt_id = ctx.parent->parent->index + 1;
		int line = ctx.parent->index + 1;

		if (prev_evt_id != evt_id || prev_line != line - 1 || prev_indent != evt.indent) {
			add_evt_entry();
		}

		switch (static_cast<lcf::rpg::EventCommand::Code>(evt.code)) {
			case Cmd::ShowMessage:
				// New message, push old one
				add_evt_entry();

				info.push_back(make_info(ctx));
				lines.push_back(Utils::RemoveControlChars(evt.string));
				break;
			case Cmd::ShowMessage_2:
				// Next message line
				if (lines.empty()) {
					// shouldn't happen
					std::cerr << "Corrupted event (Message continuation without Message start) " << evt_id << "@" << line << "\n";
				}

				lines.push_back(Utils::RemoveControlChars(evt.string));
				break;
			case Cmd::ShowChoice: {
				const auto &pevt = *reinterpret_cast<T*>(ctx.parent->obj);
				auto choices = Utils::GetChoices(pevt.event_commands, line);
				if (choices.size() + lines.size() > lines_per_message) {
					// The choice will be on a new page -> create two entries
					// Event, Page, Line
					add_evt_entry();

					info.push_back(make_info(ctx));
					info.push_back("Choice (" + std::to_string(choices.size()) + " options)");
					lines = choices;
				} else {
					// The choice is on the same page as the current message
					info.push_back("Choice starting at line " + std::to_string(lines.size() + 1) + " (" + std::to_string(choices.size()) + " options)");
					lines.insert(lines.end(), choices.begin(), choices.end());
				}
				add_evt_entry();
			}
				break;
			default:
				break;
		}

		prev_evt_id = evt_id;
		prev_line = line;
		prev_indent = evt.indent;
	});
	add_evt_entry();
}

TranslationLdb Translation::fromLDB(const std::string& filename, const std::string& encoding) {
	TranslationLdb t;

	lcf::LDB_Reader::Load(filename, encoding);

	auto chunks = { "actors", "classes", "skills", "items", "enemies", "states", "terms" };

	std::vector<std::vector<std::string>> fields = {
		{ "actors", "name", "title", "skill_name" },
		{ "classes", "name" },
		{ "skills", "name", "description", "using_message1", "using_message2" },
		{ "items", "name", "description" },
		{ "enemies", "name" },
		{ "states", "name", "message_actor", "message_enemy", "message_already", "message_affected", "message_recovery" }
	};

	// Process non-event strings
	lcf::rpg::ForEachString(lcf::Data::data, nullptr, [&](lcf::Context<lcf::DBString>& ctx) {
		if (!ctx.parent || ctx.parent->parent) {
			// Only care about entries one level deep
			return;
		}

		if (!ctx.value->empty()) {
			lcf::StringView pname = lcf::ToString(ctx.parent->name);
			lcf::StringView name = ctx.name;

			if (std::find(chunks.begin(), chunks.end(), pname) == chunks.end()) {
				return;
			}

			if (pname != "terms") {
				for (auto& field : fields) {
					if (ctx.parent->name == field[0]) {
						if (std::find(field.begin(), field.end(), name) == field.end()) {
							return;
						}
					}
				}
			}

			Entry e;
			e.original = lcf::ToString(*ctx.value);
			e.context = lcf::ToString(pname) + "." + lcf::ToString(name);
			if (ctx.parent->index > -1) {
				e.info = "ID " + std::to_string(ctx.parent->index + 1);
			}
			t.terms.addEntry(e);
		}
	});

	parseEvents<lcf::rpg::CommonEvent>(t.common_events, "commonevents", lcf::Data::data, [](lcf::Context<lcf::DBString>& ctx) {
		std::string id = "ID " + std::to_string(ctx.parent->parent->index + 1);
		std::string line = "Line " + std::to_string(ctx.parent->index + 1);
		return id + ", " + line;
	});
	parseEvents<lcf::rpg::TroopPage>(t.battle_events, "pages", lcf::Data::data, [](lcf::Context<lcf::DBString>& ctx) {
		std::string id = "ID " + std::to_string(ctx.parent->parent->parent->index + 1);
		std::string page = "Page " + std::to_string(ctx.parent->parent->index + 1);
		std::string line = "Line " + std::to_string(ctx.parent->index + 1);
		return id + ", " + page + ", " + line;
	});

	return t;
}

Translation Translation::fromLMT(const std::string &filename, const std::string &encoding) {
	Translation t;

	lcf::LMT_Reader::Load(filename, encoding);

	// Process non-event strings
	lcf::rpg::ForEachString(lcf::Data::treemap, nullptr, [&](lcf::Context<lcf::DBString>& ctx) {
		if (!ctx.parent || ctx.name != "name") {
			// Only care about "name" of map tree items
			return;
		}

		const auto &info = *reinterpret_cast<lcf::rpg::MapInfo*>(ctx.obj);

		if (info.type == lcf::rpg::TreeMap::MapType_map) {
			Entry e;
			e.original = lcf::ToString(*ctx.value);
			e.info = "ID " + std::to_string(ctx.parent->index + 1);
			t.addEntry(e);
		}
	});

	return t;
}

Translation Translation::fromLMU(const std::string& filename, const std::string& encoding) {
	lcf::rpg::Map map = *lcf::LMU_Reader::Load(filename, encoding);

	Translation t;
	parseEvents<lcf::rpg::EventPage>(t, "pages", map, [](lcf::Context<lcf::DBString>& ctx) {
		auto* event = reinterpret_cast<lcf::rpg::Event*>(ctx.parent->parent->obj);
		std::string id = "ID " + std::to_string(event->ID);
		std::string page = "Page " + std::to_string(ctx.parent->parent->index + 1);
		std::string line = "Line " + std::to_string(ctx.parent->index + 1);
		std::string pos = "Pos (" + std::to_string(event->x) + "," + std::to_string(event->y) + ")";

		return id + ", " + page + ", " + line + ", " + pos;
	});
	return t;
}

static bool starts_with(const std::string& str, const std::string& search) {
	return str.find(search) == 0;
}

static std::string extract_string(const std::string& str) {
	// Extraction of a sub-match
	std::regex base_regex(R"raw(^.*?"(.*)\n?")raw");
	std::smatch base_match;

	if (std::regex_match(str, base_match, base_regex)) {
		if (base_match.size() == 2) {
			std::ssub_match base_sub_match = base_match[1];
			std::string base = base_sub_match.str();
			return Utils::Unescape(base);
		}
	}

	return "";
}

Translation Translation::fromPO(const std::string& filename) {
	// Super simple parser.
	// Ignores header and comments but good enough for use in liblcf later...
	
	Translation t;

	std::ifstream in(filename);

	std::string line;
	bool found_header = false;
	bool parse_item = false;

	Entry e;

	while (std::getline(in, line, '\n')) {
		if (!found_header) {
			if (starts_with(line, "msgstr")) {
				found_header = true;
			}
			continue;
		}

		if (!parse_item) {
			if (starts_with(line, "msgctxt")) {
				e.context = extract_string(line);

				parse_item = true;
			} else if (starts_with(line, "msgid")) {
				parse_item = true;

				goto read_msgid;
			}
		} else {
			if (starts_with(line, "msgid")) {
			read_msgid:;
				// Parse multiply lines until empty line or msgstr is encountered
				e.original = extract_string(line);

				while (std::getline(in, line, '\n')) {
					if (line.empty() || starts_with(line, "msgstr")) {
						goto read_msgstr;
					}
					e.original += "\n" + extract_string(line);
				}
			} else if (starts_with(line, "msgstr")) {
			read_msgstr:;
				// Parse multiply lines until empty line or comment
				e.translation = extract_string(line);

				while (std::getline(in, line, '\n')) {
					if (line.empty() || starts_with(line, "#")) {
						break;
					}
					e.translation += "\n" + extract_string(line);
				}

				parse_item = false;
				t.addEntry(e);
			}
		}
	}

	return t;
}
