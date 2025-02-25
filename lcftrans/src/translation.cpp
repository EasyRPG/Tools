/*
 * Copyright (c) 2020 LcfTrans authors
 * This file is released under the MIT License
 * http://opensource.org/licenses/MIT
 */

#include "translation.h"
#include "types.h"
#include "utils.h"

#include <iostream>
#include <map>
#include <sstream>
#include <fstream>
#include <regex>
#include <lcf/context.h>
#include <lcf/rpg/eventcommand.h>
#include <lcf/ldb/reader.h>
#include <lcf/lmu/reader.h>
#include <lcf/lmt/reader.h>
#include <lcf/rpg/database.h>

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
		std::string key = e.context + "\1" + Utils::Join(e.original);

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
				for (const auto &info: e->info) {
					out << "#. " << info << "\n";
				}
			}

			if (e->fuzzy) {
				out << "#, fuzzy\n";
			}
		}
		items[s][0]->write(out);

		out << "\n";
	}
}

bool Translation::addEntry(const Entry& entry) {
	if (std::all_of(entry.original.begin(), entry.original.end(), [](const auto& e) {
		return e.empty();
	})) {
		return false;
	}
	entries.push_back(entry);
	return true;
}

const std::vector<Entry>& Translation::getEntries() const {
	return entries;
}

Translation Translation::Merge(const Translation& from) {
	auto efrom = from.getEntries();

	// Ignore strings that don't have a translation at all
	efrom.erase(
		std::remove_if(efrom.begin(), efrom.end(), [](Entry &e) { return !e.hasTranslation(); }),
	efrom.end());

	// Copy over and find stale entries (entries that are not available in the new translation anymore)
	Translation stale;
	for (auto& e_from : efrom) {
		bool found = false;
		for (auto& e_to : entries) {
			if (e_from.original == e_to.original) {
				e_to.translation = e_from.translation;
				// no dedup when parsing LCF files, don't "continue" here
				found = true;
			}
		}
		if (!found) {
			stale.addEntry(e_from);
		}
	}

	return stale;
}

Translation Translation::Match(const Translation& from, int& matches) {
	matches = 0;
	auto efrom = from.getEntries();

	auto starts_with = [](const std::string& line, const std::string& search) {
		return line.find(search) == 0;
	};

	Translation stale;
	for (auto& e_from : efrom) {
		bool found = false;
		if (!e_from.context.empty()) {
			// Match by context first
			for (auto& e_to : entries) {
				if (e_to.hasTranslation()) {
					continue;
				}

				if (e_from.context == e_to.context) {
					// Also ensure that the ID matches to reduce false-positive rate
					std::string info = Utils::Join(e_from.info, '\n');
					if (starts_with(info, "ID ")) {
						std::string info_to = Utils::Join(e_to.info, '\n');
						if (info != info_to) {
							continue;
						}
					}

					e_to.translation = e_from.original;
					++matches;
					found = true;
					break;
				}
			}
		} else {
			std::string info = Utils::Join(e_from.info, '\n');
			if (starts_with(info, "ID ")) {
				// Is a event location identifier
				// Attempt exact match
				for (auto& e_to : entries) {
					if (e_to.hasTranslation()) {
						continue;
					}

					std::string info_to = Utils::Join(e_to.info, '\n');
					if (info == info_to) {
						e_to.translation = e_from.original;
						found = true;
						++matches;
						break;
					}
				}
				// Attempt fuzzy match (Ignore line number)
				if (!found) {
					std::regex re("Line [0-9]+");
					std::string info_cp = info;
					info.clear();
					std::regex_replace(std::back_inserter(info), info_cp.begin(), info_cp.end(), re, "");
					for (auto& e_to : entries) {
						if (e_to.hasTranslation()) {
							continue;
						}

						std::string info_to_cp = Utils::Join(e_to.info, '\n');
						std::string info_to;
						std::regex_replace(std::back_inserter(info_to), info_to_cp.begin(), info_to_cp.end(), re, "");
						if (info == info_to) {
							e_to.translation = e_from.original;
							e_to.fuzzy = true;
							found = true;
							++matches;
							break;
						}
					}
				}
			}
		}

		if (!found) {
			stale.addEntry(e_from);
		}
	}

	return stale;
}

template <typename T> bool isEventCommandString(const lcf::ContextStructBase<T>&) { return false; }
bool isEventCommandString(const lcf::ContextStructBase<lcf::rpg::EventCommand>& ctx) { return ctx.name == "string"; }

template<typename T>
int getEventId(const map_event_ctx<T>& ctx) { return ctx.parent->parent->obj->ID; }
template<typename T>
int getEventId(const common_event_ctx<T>& ctx) { return ctx.parent->obj->ID; }
template<typename T>
int getEventId(const battle_event_ctx<T>& ctx) { return ctx.parent->parent->obj->ID; }

template<typename F>
struct ParseEvent {
public:
	ParseEvent(Translation& t, const F& make_info) : t(t), make_info(make_info) {}

	void add_evt_entry() {
		if (lines.empty()) {
			info.clear();
			return;
		}

		Entry e;
		e.original = lines;
		e.info = info;
		e.context = context;
		t.addEntry(e);
		lines.clear();
		info.clear();
		context.clear();
	};

	template<typename T>
	void parse(const T&) {}

	template<typename T>
	void parse(const any_event_ctx<T>& ctx) {
		int evt_id = getEventId(ctx);
		int line = ctx.parent->index + 1;

		auto indent = ctx.obj->indent;
		auto code = ctx.obj->code;
		auto estring = ctx.obj->string;

		if (prev_evt_id != evt_id || prev_line != line - 1 || prev_indent != indent) {
			add_evt_entry();
		}

		switch (static_cast<lcf::rpg::EventCommand::Code>(code)) {
			case Cmd::ShowMessage:
				// New message, push old one
				add_evt_entry();

				info.push_back(make_info(ctx));
				lines.push_back(Utils::RemoveControlChars(estring));
				break;
			case Cmd::ShowMessage_2:
				// Next message line
				if (lines.empty()) {
					// shouldn't happen
					std::cerr << "Corrupted event (Message continuation without Message start) " << evt_id << "@" << line << "\n";
				}

				lines.push_back(Utils::RemoveControlChars(estring));
				break;
			case Cmd::ShowChoice: {
				auto choices = Utils::GetChoices(ctx.parent->obj->event_commands, line);
				bool part_of_msg = !lines.empty() && choices.size() + lines.size() <= lines_per_message;
				int starting_at = lines.size() + 1;

				if (part_of_msg) {
					info.push_back("Contains choice at line " + std::to_string(starting_at) + " (" + std::to_string(choices.size()) + " options)");
				}

				add_evt_entry();

				info.push_back(make_info(ctx));
				info.push_back("Choice (" + std::to_string(choices.size()) + " options" + (part_of_msg ? ", embedded in a message)" : ")"));
				lines = choices;
				add_evt_entry();
			}
				break;
			case Cmd::ChangeHeroName:
				add_evt_entry();
				info.push_back(make_info(ctx));
				info.push_back("ChangeHeroName (Actor " + std::to_string(ctx.obj->parameters[0]) + ")");
				lines.push_back(Utils::RemoveControlChars(estring));
				context = "actors.name";
				add_evt_entry();
				break;
			case Cmd::ChangeHeroTitle:
				add_evt_entry();
				info.push_back(make_info(ctx));
				info.push_back("ChangeHeroTitle (Actor " + std::to_string(ctx.obj->parameters[0]) + ")");
				lines.push_back(Utils::RemoveControlChars(estring));
				context = "actors.title";
				add_evt_entry();
				break;
			case Cmd::ConditionalBranch:
				// Condition: Actor Name is
				if (ctx.obj->parameters[0] == 5 && ctx.obj->parameters[2] == 1) {
					add_evt_entry();
					info.push_back(make_info(ctx));
					info.push_back("Condition (Actor Name = " + lcf::ToString(estring) + ")");
					lines.push_back(Utils::RemoveControlChars(estring));
					context = "actors.name";
					add_evt_entry();
				}
				break;
			case Cmd::Maniac_ShowStringPicture: {
				// Show String Picture
				add_evt_entry();

				if (!estring.empty() && estring[0] == '\x01') {
					std::string term = "";

					for (size_t i = 1; i < estring.size(); ++i) {
						char c = estring[i];
						if (c == '\x01' || c == '\x02' || c == '\x03') {
							break;
						}
						term += c;
					}

					info.push_back(make_info(ctx));
					info.push_back("Show String Picture");
					for (auto& line: Utils::Split(term, '\n')) {
						lines.push_back(Utils::RemoveControlChars(line));
					}
					context = "strpic";
					add_evt_entry();
				}
				break;
			}
			default:
				break;
		}

		prev_evt_id = evt_id;
		prev_line = line;
		prev_indent = indent;
	}

private:
	std::vector<std::string> lines;
	std::vector<std::string> info;
	std::string context;
	int prev_evt_id = 0;
	int prev_line = 0;
	int prev_indent = 0;

	Translation& t;
	const F& make_info;
};

template<typename ParentType, typename Root, typename F>
static void parseEvents(Translation& t, Root& root, const F& make_info) {
	// Read events
	ParseEvent<decltype(make_info)> p { t, make_info };

	lcf::rpg::ForEachString(root, [&](const auto& val, const auto& ctx) {
		if (!ctx.parent ||
				!std::is_same<decltype(ctx.parent->obj), std::add_pointer_t<ParentType>>::value ||
				!isEventCommandString(ctx)) {
			return;
		}

		p.parse(ctx);
	});
	p.add_evt_entry();
}

template <typename T>
std::string makeEventInfoString(const map_event_ctx<T>& ctx) {
	const auto& event = ctx.parent->parent->obj;
	std::string id = "ID " + std::to_string(getEventId(ctx));
	std::string page = "Page " + std::to_string(ctx.parent->obj->ID);
	std::string line = "Line " + std::to_string(ctx.parent->index + 1);
	std::string pos = "Pos (" + std::to_string(event->x) + "," + std::to_string(event->y) + ")";
	return id + ", " + page + ", " + line + ", " + pos;
}

template <typename T>
std::string makeEventInfoString(const common_event_ctx<T>& ctx) {
	std::string id = "ID " + std::to_string(getEventId(ctx));
	std::string line = "Line " + std::to_string(ctx.parent->index + 1);
	return id + ", " + line;
}

template <typename T>
std::string makeEventInfoString(const battle_event_ctx<T>& ctx) {
	std::string id = "ID " + std::to_string(getEventId(ctx));
	std::string page = "Page " + std::to_string(ctx.parent->obj->ID);
	std::string line = "Line " + std::to_string(ctx.parent->index + 1);
	return id + ", " + page + ", " + line;
}

TranslationLdb Translation::fromLDB(const std::string& filename, const std::string& encoding) {
	TranslationLdb t;

	auto db = lcf::LDB_Reader::Load(filename, encoding);

	if (!db) {
		std::cerr << "Error loading database " << filename << "\n";
		return t;
	}

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
	lcf::rpg::ForEachString(*db, [&](const auto& val, const auto& ctx) {
		if (!ctx.parent || ctx.parent->parent) {
			// Only care about entries one level deep
			return;
		}

		if (!val.empty()) {
			lcf::StringView pname = ctx.parent->name;
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
			e.original.emplace_back(lcf::ToString(val));
			e.context = lcf::ToString(pname) + "." + lcf::ToString(name);
			if (ctx.parent->index > -1) {
				e.info.emplace_back("ID " + std::to_string(ctx.parent->index + 1));
			}
			t.terms.addEntry(e);
		}
	});

	parseEvents<lcf::rpg::CommonEvent>(t.common_events, *db, [](auto& ctx) {
		return makeEventInfoString(ctx);
	});
	parseEvents<lcf::rpg::TroopPage>(t.battle_events, *db, [](auto& ctx) {
		return makeEventInfoString(ctx);
	});

	return t;
}

template <typename T, typename U>
bool isMapType(const T&) { return false; }

bool isMapType(const lcf::rpg::MapInfo& info) {
	return info.type == lcf::rpg::TreeMap::MapType_map;
}

Translation Translation::fromLMT(const std::string &filename, const std::string &encoding) {
	Translation t;

	auto tree = lcf::LMT_Reader::Load(filename, encoding);

	if (!tree) {
		std::cerr << "Error loading map tree " << filename << "\n";
		return t;
	}

	// Process non-event strings
	lcf::rpg::ForEachString(*tree, [&](const auto& val, const auto& ctx) {
		if (!ctx.parent || ctx.name != "name") {
			// Only care about "name" of map tree items
			return;
		}

		if (isMapType(*ctx.obj)) {
			Entry e;
			e.original.emplace_back(lcf::ToString(val));
			e.info.emplace_back("ID " + std::to_string(ctx.parent->index + 1));
			t.addEntry(e);
		}
	});

	return t;
}

Translation Translation::fromLMU(const std::string& filename, const std::string& encoding) {
	lcf::rpg::Map map = *lcf::LMU_Reader::Load(filename, encoding);

	Translation t;
	parseEvents<lcf::rpg::EventPage>(t, map, [](const auto& ctx) {
		return makeEventInfoString(ctx);
	});
	return t;
}

Translation Translation::fromPO(const std::string& filename) {
	// Super simple parser.
	// Only parses msgstr, msgid, msgctx and #.

	Translation t;

	std::ifstream in(filename);

	std::string line;
	lcf::StringView line_view;
	bool found_header = false;
	bool parse_item = false;
	int line_number = 0;

	Entry e;

	auto extract_string = [&](int offset) -> std::string {
		if (offset >= line_view.size()) {
			std::cerr << "Parse error (Line " << line_number << ") is empty\n";
			return "";
		}

		std::stringstream out;
		bool slash = false;
		bool first_quote = false;

		for (char c : line_view.substr(offset)) {
			if (!first_quote) {
				if (c == ' ') {
					continue;
				} else if (c == '"') {
					first_quote = true;
					continue;
				}
				std::cerr << "Parse error (Line " << line_number << "): Expected \", got " << c << ": " << line << "\n";
				return "";
			}

			if (!slash && c == '\\') {
				slash = true;
			} else if (slash) {
				slash = false;
				switch (c) {
					case '\\':
						out << c;
						break;
					case 'n':
						out << '\n';
						break;
					case '"':
						out << '"';
						break;
					default:
						std::cerr << "Parse error (Line " << line_number << "): Expected \\, \\n or \", got " << c << ": " << line << "\n";
						break;
				}
			} else {
				// no-slash
				if (c == '"') {
					// done
					return out.str();
				}
				out << c;
			}
		}

		std::cerr << "Parse error (Line " << line_number << "): Unterminated line: " << line << "\n";
		return out.str();
	};

	auto read_msgctx = [&]() {
		e.context = extract_string(7);
	};

	auto read_msgstr = [&]() {
		// Parse multiply lines until empty line or comment
		std::string msgstr = extract_string(6);

		while (Utils::ReadLine(in, line)) {
			line_view = Utils::TrimWhitespace(line);
			++line_number;
			if (line_view.empty() || line_view.starts_with("#")) {
				break;
			}
			msgstr += extract_string(0);
		}

		parse_item = false;
		e.translation = Utils::Split(msgstr);
		t.addEntry(e);
		e = Entry();
	};

	auto read_msgid = [&]() {
		// Parse multiply lines until empty line or msgstr is encountered
		std::string msgid = extract_string(5);

		while (Utils::ReadLine(in, line)) {
			line_view = Utils::TrimWhitespace(line);
			++line_number;
			if (line_view.empty() || line_view.starts_with("msgstr")) {
				e.original = Utils::Split(msgid);
				read_msgstr();
				return;
			}
			msgid += extract_string(0);
		}
		e.original = Utils::Split(msgid);
	};

	auto read_info = [&]() {
		if (line.length() <= 3) {
			return;
		}

		// Parse multiply lines until empty line, msgctxt or msgid is encountered
		e.info.push_back(line.substr(3));

		while (Utils::ReadLine(in, line)) {
			line_view = Utils::TrimWhitespace(line);
			++line_number;
			if (line.empty() || line_view.starts_with("msgctx") || line_view.starts_with("msgid")) {
				if (line_view.starts_with("msgctx")) {
					read_msgctx();
				} else if (line_view.starts_with("msgid")) {
					read_msgid();
				}
				return;
			}
			else if (line_view.starts_with("#.")) {
				if (line.length() > 3) {
					e.info.push_back(line.substr(3));
				}
			} else {
				std::cerr << "Parse error (Line " << line_number << ") " << line << " (" << line << "). Expected #., msgctx or msgid\n";
				return;
			}
		}
	};

	while (Utils::ReadLine(in, line)) {
		line_view = Utils::TrimWhitespace(line);
		++line_number;
		if (!found_header) {
			if (line_view.starts_with("msgstr")) {
				found_header = true;
			}
			continue;
		}

		if (!parse_item) {
			if (line_view.starts_with("#.")) {
				parse_item = true;
				read_info();
			} else if (line_view.starts_with("msgctxt")) {
				read_msgctx();
				parse_item = true;
			} else if (line_view.starts_with("msgid")) {
				parse_item = true;
				read_msgid();
			}
		} else {
			if (line_view.starts_with("msgid")) {
				read_msgid();
			} else if (line_view.starts_with("msgstr")) {
				read_msgstr();
			}
		}
	}

	return t;
}
