/*
* Copyright (c) 2016 LcfTrans authors
* This file is released under the MIT License
* http://opensource.org/licenses/MIT
*/

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

#include "translation.h"

#ifdef _WIN32
#include "dirent_win.h"
#else
#include <dirent.h>
#endif

#define DATABASE_FILE "rpg_rt.ldb"

void DumpLdb(const std::string& filename);
void DumpLmu(const std::string& filename);

bool endsWith(std::string const & val, std::string const & ending);
std::string toLower(std::string const & instr);

int main(int argc, char** argv)
{
	/*if (argc <= 1)
	{
		std::cerr << "Lcf2Xml - Converts RPG Maker 2k/2k3 Files to XML and vice versa" << std::endl;
		std::cerr << "Usage: lcf2xml [FILE] [OUTFILE]" << std::endl;
		//std::cerr << "If OUTFILE is missing the resulting file is named FILE with the extension replaced" << std::endl;
		return 1;
	}*/

	std::string infile;

	DIR *dirHandle;
	struct dirent * dirEntry;

	dirHandle = opendir(".");
	if (dirHandle) {
		while (0 != (dirEntry = readdir(dirHandle))) {
			std::string name = dirEntry->d_name;
			std::string lname = toLower(name);

			if (lname == DATABASE_FILE) {
				std::cout << "Parsing Database " << name << std::endl;
				DumpLdb(name);
			}
			else if (endsWith(lname, ".lmu")) {
				std::cout << "Parsing Map " << name << std::endl;
				DumpLmu(name);
			}
		}
		closedir(dirHandle);
	}

}

// https://stackoverflow.com/questions/874134/
bool endsWith(std::string const & val, std::string const & ending)
{
	if (ending.size() > val.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), val.rbegin());
}

std::string toLower(std::string const & in) {
	std::string data = in;

	std::transform(data.begin(), data.end(), data.begin(), static_cast<int(*)(int)>(std::tolower));

	return data;
}

// https://stackoverflow.com/questions/2896600/
std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

/** Returns the filename (without extension). */
std::string GetFilename(const std::string& str)
{
	std::string s = str;
#ifdef _WIN32
	std::replace(s.begin(), s.end(), '\\', '/');
#endif
	// Extension
	size_t found = s.find_last_of(".");
	if (found != std::string::npos)
	{
		s = s.substr(0, found);
	}

	// Filename
	found = s.find_last_of("/");
	if (found == std::string::npos)	{
		return s;
	}

	s = s.substr(found + 1);
	return s;
}

void DumpLdb(const std::string& filename) {
	Translation* t = Translation::fromLDB(filename, "1252");

	std::ofstream outfile(GetFilename(filename) + ".po");

	t->write(outfile);
}

void DumpLmu(const std::string& filename) {
	Translation* t = Translation::fromLMU(filename, "1252");

	std::ofstream outfile(GetFilename(filename) + ".po");

	t->write(outfile);
}

/*
void getStrings(std::vector<std::string>& ret_val, const std::vector<RPG::EventCommand>& list, int index) {
	// Let's find the choices
	int current_indent = list[index + 1].indent;
	unsigned int index_temp = index + 1;
	std::vector<std::string> s_choices;
	while (index_temp < list.size()) {
		if ((list[index_temp].code == Cmd::ShowChoiceOption) && (list[index_temp].indent == current_indent)) {
			// Choice found
			s_choices.push_back(list[index_temp].string);
		}
		// If found end of show choice command
		if (((list[index_temp].code == Cmd::ShowChoiceEnd) && (list[index_temp].indent == current_indent)) ||
			// Or found Cancel branch
			((list[index_temp].code == Cmd::ShowChoiceOption) && (list[index_temp].indent == current_indent) &&
			(list[index_temp].string == ""))) {

			break;
		}
		// Move on to the next command
		index_temp++;
	}
	ret_val.swap(s_choices);
}

void writeHeader(std::ofstream& outfile) {
	outfile << "msgid \"\"" << std::endl;
	outfile << "msgstr \"\"" << std::endl;
	outfile << "\"Project-Id-Version: GAME_NAME 1.0\\n\"" << std::endl;
	outfile << "\"Report-Msgid-Bugs-To: YOUR WEBSITE OR MAIL\n" << std::endl;
	outfile << "\"POT-Creation-Date: \\n\"" << std::endl;
	outfile << "\"PO-Revision-Date: \\n\"" << std::endl;
	outfile << "\"Language-Team: LANGUAGE <mail@your.address>\\n\"" << std::endl;
	outfile << "\"Language: LANGUAGE_CODE\\n\"" << std::endl;
	outfile << "\"MIME-Version: 1.0\\n\"" << std::endl;
	outfile << "\"Content-Type: text/plain; charset=UTF-8\\n\"" << std::endl;
	outfile << "\"Content-Transfer-Encoding: 8bit\\n\"" << std::endl;
	outfile << "\"Plural-Forms: nplurals=INTEGER; plural=EXPRESSION;\n\"" << std::endl;
	outfile << "\"X-Generator: LcfTrans\\n\"" << std::endl;
	outfile << std::endl;
}

void write(std::ofstream& outfile, const std::string& ref, const std::string& msg) {
	if (msg.empty()) {
		return;
	}

	std::string msgid = ReplaceAll(msg, "\\", "\\\\");
	msgid = ReplaceAll(msgid, "\"", "\\\"");

	outfile << "#. " << ref << std::endl;
	outfile << "msgid \"" << msgid << "\"" << std::endl;
	outfile << "msgstr \"" << msgid << "\"" << std::endl;
	outfile << std::endl;
}

void writeprefix(std::ofstream& outfile, const std::string& ref, const std::string& msg) {
	if (msg.empty()) {
		return;
	}

	write(outfile, ref, "%1" + msg);
}

void writeMsgBegin(std::ofstream& outfile, const std::string& ref, const std::string& msg, std::vector<std::string>& lines) {
	std::string msgid = ReplaceAll(msg, "\\", "\\\\");
	msgid = ReplaceAll(msgid, "\"", "\\\"");

	outfile << "#. " << ref << std::endl;
	outfile << "msgid \"\"" << std::endl;
	outfile << "\"" << msgid << "\\n\"" << std::endl;
	
	lines.push_back(msgid);
}

void writeMsgAppend(std::ofstream& outfile, const std::string& msg, std::vector<std::string>& lines) {
	std::string msgid = ReplaceAll(msg, "\\", "\\\\");
	msgid = ReplaceAll(msgid, "\"", "\\\"");

	outfile << "\"" << msgid << "\\n\"" << std::endl;

	lines.push_back(msgid);
}

void writeMsgEnd(std::ofstream& outfile, std::vector<std::string>& lines) {
	outfile << "msgstr \"\"" << std::endl;

	for (size_t i = 0; i < lines.size(); ++i) {
		outfile << "\"" << lines[i] << "\\n\"" << std::endl;
	}
	lines.clear();

	outfile << std::endl;
}

void DumpLdb(const std::string& filename) {
	LDB_Reader::Load(filename, "1252");

	std::ofstream outfile(GetFilename(filename) + ".po");

	writeHeader(outfile);

	char ref[255];

	for (size_t i = 0; i < Data::actors.size(); ++i) {
		const RPG::Actor& actor = Data::actors[i];
		int count = i + 1;

		sprintf(ref, "Actor %d: Name", count);
		write(outfile, ref, actor.name);

		sprintf(ref, "Actor %d: Title", count);
		write(outfile, ref, actor.title);

		sprintf(ref, "Actor %d: Skill name", count);
		write(outfile, ref, actor.skill_name);
	}

	for (size_t i = 0; i < Data::classes.size(); ++i) {
		const RPG::Class& cls = Data::classes[i];
		int count = i + 1;

		sprintf(ref, "Class %d: Name", count);
		write(outfile, ref, cls.name);
	}

	for (size_t i = 0; i < Data::skills.size(); ++i) {
		const RPG::Skill& skill = Data::skills[i];
		int count = i + 1;

		sprintf(ref, "Skill %d: Name", count);
		write(outfile, ref, skill.name);

		sprintf(ref, "Skill %d: Description", count);
		write(outfile, ref, skill.description);

		sprintf(ref, "Skill %d: Using message 1", count);
		writeprefix(outfile, ref, skill.using_message1);

		sprintf(ref, "%d: Using message 2", count);
		write(outfile, ref, skill.using_message2);
	}

	for (size_t i = 0; i < Data::items.size(); ++i) {
		const RPG::Item& item = Data::items[i];
		int count = i + 1;

		sprintf(ref, "Item %d: Name", count);
		write(outfile, ref, item.name);

		sprintf(ref, "Item %d: Description", count);
		write(outfile, ref, item.description);
	}

	for (size_t i = 0; i < Data::enemies.size(); ++i) {
		const RPG::Enemy& enemy = Data::enemies[i];
		int count = i + 1;

		sprintf(ref, "Enemy %d: Name", count);
		write(outfile, ref, enemy.name);
	}

	for (size_t i = 0; i < Data::states.size(); ++i) {
		const RPG::State& state = Data::states[i];
		int count = i + 1;

		sprintf(ref, "State %d: Name", count);
		write(outfile, ref, state.name);

		sprintf(ref, "State %d: Message actor", count);
		writeprefix(outfile, ref, state.message_actor);

		sprintf(ref, "State %d: Message enemy", count);
		writeprefix(outfile, ref, state.message_enemy);

		sprintf(ref, "State %d: Message already", count);
		writeprefix(outfile, ref, state.message_already);

		sprintf(ref, "State %d: Message affected", count);
		writeprefix(outfile, ref, state.message_affected);

		sprintf(ref, "State %d: Message recovery", count);
		writeprefix(outfile, ref, state.message_recovery);
	}

	for (size_t i = 0; i < Data::battlecommands.commands.size(); ++i) {
		const RPG::BattleCommand& cmd = Data::battlecommands.commands[i];
		int count = i + 1;

		sprintf(ref, "Battle Command %d: Name", count);
		write(outfile, ref, cmd.name);
	}

	write(outfile, "Term: Battle encounter", "%1" + Data::terms.encounter);
	write(outfile, "Term: Battle surprise attack", Data::terms.special_combat);
	write(outfile, "Term: Battle escape success", Data::terms.escape_success);
	write(outfile, "Term: Battle escape failed", Data::terms.escape_failure);
	write(outfile, "Term: Battle victory", Data::terms.victory);
	write(outfile, "Term: Battle defeat", Data::terms.defeat);
	write(outfile, "Term: Battle exp received", Data::terms.exp_received);
	write(outfile, "Term: Battle gold received", Data::terms.gold_recieved_a + " %1" + Data::terms.gold_recieved_b);
	write(outfile, "Term: Battle item received", "%1" + Data::terms.item_recieved);
	write(outfile, "Term: Battle attack", "%1" + Data::terms.attacking);
	write(outfile, "Term: Battle ally critical hit", Data::terms.actor_critical);
	write(outfile, "Term: Battle enemy critical hit", Data::terms.enemy_critical);
	write(outfile, "Term: Battle defending", "%1" + Data::terms.defending);
	write(outfile, "Term: Battle observing", "%1" + Data::terms.observing);
	write(outfile, "Term: Battle focus", "%1" + Data::terms.focus);
	write(outfile, "Term: Battle autodestruct", "%1" + Data::terms.autodestruction);
	write(outfile, "Term: Battle enemy escape", "%1" + Data::terms.enemy_escape);
	write(outfile, "Term: Battle enemy transform", "%1" + Data::terms.enemy_transform);
	write(outfile, "Term: Battle enemy damaged", "%1 - %2" + Data::terms.enemy_damaged);
	write(outfile, "Term: Battle enemy undamaged", "%1" + Data::terms.enemy_undamaged);
	write(outfile, "Term: Battle actor damaged", "%1 - %2" + Data::terms.actor_damaged);
	write(outfile, "Term: Battle actor undamaged", "%1" + Data::terms.actor_undamaged);
	write(outfile, "Term: Battle skill failure a", "%1" + Data::terms.skill_failure_a);
	write(outfile, "Term: Battle skill failure b", "%1" + Data::terms.skill_failure_b);
	write(outfile, "Term: Battle skill failure c", "%1" + Data::terms.skill_failure_c);
	write(outfile, "Term: Battle dodge", "%1" + Data::terms.dodge);
	write(outfile, "Term: Battle use item", "%1" + Data::terms.use_item);
	write(outfile, "??? Term: Battle hp recovery", Data::terms.hp_recovery);
	write(outfile, "Term: Battle parameter increase", Data::terms.parameter_increase);
	write(outfile, "Term: Battle parameter decrease", Data::terms.parameter_decrease);
	write(outfile, "Term: Battle actor_hp_absorbed", Data::terms.actor_hp_absorbed);
	write(outfile, "Term: Battle enemy_hp_absorbed", Data::terms.enemy_hp_absorbed);
	write(outfile, "Term: Battle resistance_increase", Data::terms.resistance_increase);
	write(outfile, "Term: Battle resistance_decrease", Data::terms.resistance_decrease);
	write(outfile, "Term: Battle level_up", Data::terms.level_up);
	write(outfile, "Term: Battle skill_learned", Data::terms.skill_learned);
	write(outfile, "Term: battle_start", Data::terms.battle_start);
	write(outfile, "Term: miss", Data::terms.miss);
	write(outfile, "Term: shop_greeting1", Data::terms.shop_greeting1);
	write(outfile, "Term: shop_regreeting1", Data::terms.shop_regreeting1);
	write(outfile, "Term: shop_buy1", Data::terms.shop_buy1);
	write(outfile, "Term: shop_sell1", Data::terms.shop_sell1);
	write(outfile, "Term: shop_leave1", Data::terms.shop_leave1);
	write(outfile, "Term: shop_buy_select1", Data::terms.shop_buy_select1);
	write(outfile, "Term: shop_buy_number1", Data::terms.shop_buy_number1);
	write(outfile, "Term: shop_purchased1", Data::terms.shop_purchased1);
	write(outfile, "Term: shop_sell_select1", Data::terms.shop_sell_select1);
	write(outfile, "Term: shop_sell_number1", Data::terms.shop_sell_number1);
	write(outfile, "Term: shop_sold1", Data::terms.shop_sold1);
	write(outfile, "Term: shop_greeting2", Data::terms.shop_greeting2);
	write(outfile, "Term: shop_regreeting2", Data::terms.shop_regreeting2);
	write(outfile, "Term: shop_buy2", Data::terms.shop_buy2);
	write(outfile, "Term: shop_sell2", Data::terms.shop_sell2);
	write(outfile, "Term: shop_leave2", Data::terms.shop_leave2);
	write(outfile, "Term: shop_buy_select2", Data::terms.shop_buy_select2);
	write(outfile, "Term: shop_buy_number2", Data::terms.shop_buy_number2);
	write(outfile, "Term: shop_purchased2", Data::terms.shop_purchased2);
	write(outfile, "Term: shop_sell_select2", Data::terms.shop_sell_select2);
	write(outfile, "Term: shop_sell_number2", Data::terms.shop_sell_number2);
	write(outfile, "Term: shop_sold2", Data::terms.shop_sold2);
	write(outfile, "Term: shop_greeting3", Data::terms.shop_greeting3);
	write(outfile, "Term: shop_regreeting3", Data::terms.shop_regreeting3);
	write(outfile, "Term: shop_buy3", Data::terms.shop_buy3);
	write(outfile, "Term: shop_sell3", Data::terms.shop_sell3);
	write(outfile, "Term: shop_leave3", Data::terms.shop_leave3);
	write(outfile, "Term: shop_buy_select3", Data::terms.shop_buy_select3);
	write(outfile, "Term: shop_buy_number3", Data::terms.shop_buy_number3);
	write(outfile, "Term: shop_purchased3", Data::terms.shop_purchased3);
	write(outfile, "Term: shop_sell_select3", Data::terms.shop_sell_select3);
	write(outfile, "Term: shop_sell_number3", Data::terms.shop_sell_number3);
	write(outfile, "Term: shop_sold3", Data::terms.shop_sold3);
	write(outfile, "Term: inn_a_greeting_1", Data::terms.inn_a_greeting_1);
	write(outfile, "Term: inn_a_greeting_2", Data::terms.inn_a_greeting_2);
	write(outfile, "Term: inn_a_greeting_3", Data::terms.inn_a_greeting_3);
	write(outfile, "Term: inn_a_accept", Data::terms.inn_a_accept);
	write(outfile, "Term: inn_a_cancel", Data::terms.inn_a_cancel);
	write(outfile, "Term: inn_b_greeting_1", Data::terms.inn_b_greeting_1);
	write(outfile, "Term: inn_b_greeting_2", Data::terms.inn_b_greeting_2);
	write(outfile, "Term: inn_b_greeting_3", Data::terms.inn_b_greeting_3);
	write(outfile, "Term: inn_b_accept", Data::terms.inn_b_accept);
	write(outfile, "Term: inn_b_cancel", Data::terms.inn_b_cancel);
	write(outfile, "Term: possessed_items", Data::terms.possessed_items);
	write(outfile, "Term: equipped_items", Data::terms.equipped_items);
	write(outfile, "Term: gold", Data::terms.gold);
	write(outfile, "Term: battle_fight", Data::terms.battle_fight);
	write(outfile, "Term: battle_auto", Data::terms.battle_auto);
	write(outfile, "Term: battle_escape", Data::terms.battle_escape);
	write(outfile, "Term: command_attack", Data::terms.command_attack);
	write(outfile, "Term: command_defend", Data::terms.command_defend);
	write(outfile, "Term: command_item", Data::terms.command_item);
	write(outfile, "Term: command_skill", Data::terms.command_skill);
	write(outfile, "Term: menu_equipment", Data::terms.menu_equipment);
	write(outfile, "Term: menu_save", Data::terms.menu_save);
	write(outfile, "Term: menu_quit", Data::terms.menu_quit);
	write(outfile, "Term: new_game", Data::terms.new_game);
	write(outfile, "Term: load_game", Data::terms.load_game);
	write(outfile, "Term: exit_game", Data::terms.exit_game);
	write(outfile, "Term: status", Data::terms.status);
	write(outfile, "Term: row", Data::terms.row);
	write(outfile, "Term: order", Data::terms.order);
	write(outfile, "Term: wait_on", Data::terms.wait_on);
	write(outfile, "Term: wait_off", Data::terms.wait_off);
	write(outfile, "Term: level", Data::terms.level);
	write(outfile, "Term: health_points", Data::terms.health_points);
	write(outfile, "Term: spirit_points", Data::terms.spirit_points);
	write(outfile, "Term: normal_status", Data::terms.normal_status);
	write(outfile, "Term: exp_short", Data::terms.exp_short);
	write(outfile, "Term: lvl_short", Data::terms.lvl_short);
	write(outfile, "Term: hp_short", Data::terms.hp_short);
	write(outfile, "Term: sp_short", Data::terms.sp_short);
	write(outfile, "Term: sp_cost", Data::terms.sp_cost);
	write(outfile, "Term: attack", Data::terms.attack);
	write(outfile, "Term: defense", Data::terms.defense);
	write(outfile, "Term: spirit", Data::terms.spirit);
	write(outfile, "Term: agility", Data::terms.agility);
	write(outfile, "Term: weapon", Data::terms.weapon);
	write(outfile, "Term: shield", Data::terms.shield);
	write(outfile, "Term: armor", Data::terms.armor);
	write(outfile, "Term: helmet", Data::terms.helmet);
	write(outfile, "Term: accessory", Data::terms.accessory);
	write(outfile, "Term: save_game_message", Data::terms.save_game_message);
	write(outfile, "Term: load_game_message", Data::terms.load_game_message);
	write(outfile, "Term: file", Data::terms.file);
	write(outfile, "Term: exit_game_message", Data::terms.exit_game_message);
	write(outfile, "Term: yes", Data::terms.yes);
	write(outfile, "Term: no", Data::terms.no);

	bool has_message = false;
	std::vector<std::string> lines;
	std::vector<std::string> choices;
	
	// TODO: Change Hero Name
	// TODO: Change Hero Degree

	// This needs a po class

	// Creates a new entry if msgctxt and msgid missing, ref as #.
	// If exists and msgstr != new -> assert, new line #. ref
	
	// Entries are stored in std::vector<msg_entry>, map not worth it

	// struct msg_entry {
	// string msgctxt
	// string msgid;
	// string msgstr;
	// std::vector<string> ref;
	// }
	
	// insert(msgctxt, msgid, msgstr, ref)

	// remove(msgid, msgstr, ref) -> true removed, false not found

	// get(msgctxt, msgid, *found) -> str or empty

	// load(filename)

	// save(filename)

	for (size_t i = 0; i < Data::commonevents.size(); ++i) {
		int evt_count = i + 1;

		const std::vector<RPG::EventCommand>& events = Data::commonevents[i].event_commands;
		for (size_t j = 0; j < events.size(); ++j) {
			const RPG::EventCommand& evt = events[j];
			int line_count = j + 1;

			sprintf(ref, "Common Event %d, Line %d", evt_count, line_count);

			switch (evt.code) {
			case Cmd::ShowMessage:
				if (has_message) {
					// New message
					writeMsgEnd(outfile, lines);
				}

				has_message = true;
				writeMsgBegin(outfile, ref, evt.string, lines);

				break;
			case Cmd::ShowMessage_2:
				if (has_message) {
					// Next message line
					writeMsgAppend(outfile, evt.string, lines);
				}
				else {
					// Shouldn't happen
					has_message = true;
					writeMsgBegin(outfile, ref, evt.string, lines);
				}

				break;
			case Cmd::ShowChoice:
				if (has_message) {
					has_message = false;
					writeMsgEnd(outfile, lines);
				}

				sprintf(ref, "Common Event %d, Line %d (Choice)", evt_count, line_count);

				choices.clear();
				getStrings(choices, events, j);
				if (!choices.empty()) {
					writeMsgBegin(outfile, ref, choices[0], lines);

					for (size_t choice_i = 1; choice_i < choices.size(); ++choice_i) {
						writeMsgAppend(outfile, choices[choice_i], lines);
					}

					writeMsgEnd(outfile, lines);
				}

				break;
			default:
				if (has_message) {
					has_message = false;
					writeMsgEnd(outfile, lines);
				}
				break;
			}
		}
	}
}

void DumpLmu(const std::string& filename) {
	RPG::Map map = *LMU_Reader::Load(filename, "1252");

	std::ofstream outfile(GetFilename(filename) + ".po");

	writeHeader(outfile);

	char ref[255];

	bool has_message = false;
	std::vector<std::string> lines;
	std::vector<std::string> choices;

	for (size_t i = 0; i < map.events.size(); ++i) {
		const RPG::Event rpg_evt = map.events[i];

		for (size_t j = 0; j < rpg_evt.pages.size(); ++j) {
			int page_count = j + 1;
			const RPG::EventPage& page = rpg_evt.pages[j];

			for (size_t k = 0; k < page.event_commands.size(); ++k) {
				const RPG::EventCommand& evt = page.event_commands[k];
				int line_count = k + 1;

				sprintf(ref, "Event %d, Page %d, Line %d", rpg_evt.ID, page_count, line_count);

				switch (evt.code) {
				case Cmd::ShowMessage:
					if (has_message) {
						// New message
						writeMsgEnd(outfile, lines);
					}

					has_message = true;
					writeMsgBegin(outfile, ref, evt.string, lines);

					break;
				case Cmd::ShowMessage_2:
					if (has_message) {
						// Next message line
						writeMsgAppend(outfile, evt.string, lines);
					}
					else {
						// Shouldn't happen
						has_message = true;
						writeMsgBegin(outfile, ref, evt.string, lines);
					}

					break;
				case Cmd::ShowChoice:
					if (has_message) {
						has_message = false;
						writeMsgEnd(outfile, lines);
					}

					sprintf(ref, "Event %d, Page %d, Line %d (Choice)", rpg_evt.ID, page_count, line_count);

					choices.clear();
					getStrings(choices, page.event_commands, j);
					if (!choices.empty()) {
						writeMsgBegin(outfile, ref, choices[0], lines);

						for (size_t choice_i = 1; choice_i < choices.size(); ++choice_i) {
							writeMsgAppend(outfile, choices[choice_i], lines);
						}

						writeMsgEnd(outfile, lines);
					}

					break;
				default:
					if (has_message) {
						has_message = false;
						writeMsgEnd(outfile, lines);
					}
					break;
				}
			}
		}
	}
}
*/