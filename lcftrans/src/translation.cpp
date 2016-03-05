#include "translation.h"

#include <regex>
#include <sstream>
#include "data.h"
#include "command_codes.h"

static std::string escape(const std::string& str) {
	static std::regex quotation(R"raw(("))raw");
	static std::regex backslash(R"raw((\\))raw");

	return std::regex_replace(std::regex_replace(str, quotation, "\"$1"), backslash, "\\$1");
}

static std::string unescape(const std::string& str) {
	static std::regex quotation(R"raw((""))raw");
	static std::regex backslash(R"raw((\\\\))raw");

	return std::regex_replace(std::regex_replace(str, quotation, "\""), backslash, "\\");
}

static std::string formatLines(const std::vector<std::string> lines) {
	std::string ret;
	for (const std::string& s : lines) {
		ret += s + "\\n";
	}
	return ret;
}

static void getStrings(std::vector<std::string>& ret_val, const std::vector<RPG::EventCommand>& list, int index) {
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

Translation::Translation() {
}

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
			if (!e->info.empty()) {
				std::stringstream ss(e->info);

				std::string info;
				while (std::getline(ss, info, '\n')) {
					out << "#. " << info << std::endl;
				}
			}
		}
		items[s][0]->write(out);

		out << std::endl;
	}
}

bool Translation::addEntry(const Entry& entry) {
	if (entry.original.empty()) {
		return false;
	}
	entries.push_back(entry);
	return true;
}

Translation* Translation::fromLDB(const std::string& filename, const std::string& encoding) {
	Translation* t = new Translation();

	LDB_Reader::Load(filename, "1252");

	Entry e;

	for (size_t i = 0; i < Data::actors.size(); ++i) {
		const RPG::Actor& actor = Data::actors[i];

		e.original = actor.name;
		e.context = "actor.name";
		e.info = "Actor " + std::to_string(i + 1) + ": Name";
		t->addEntry(e);

		e.original = actor.title;
		e.context = "actor.title";
		e.info = "Actor " + std::to_string(i + 1) + ": Title";
		t->addEntry(e);

		e.original = actor.skill_name;
		e.context = "actor.skill_name";
		e.info = "Actor " + std::to_string(i + 1) + ": Skill name";
		t->addEntry(e);
	}

	for (size_t i = 0; i < Data::classes.size(); ++i) {
		const RPG::Class& cls = Data::classes[i];

		e.original = cls.name;
		e.context = "cls.name";
		e.info = "Class " + std::to_string(i + 1) + ": Name";
		t->addEntry(e);
	}

	for (size_t i = 0; i < Data::skills.size(); ++i) {
		const RPG::Skill& skill = Data::skills[i];

		e.original = skill.name;
		e.context = "skill.name";
		e.info = "Skill " + std::to_string(i + 1) + ": Name";
		t->addEntry(e);

		e.original = skill.description;
		e.context = "skill.description";
		e.info = "Skill " + std::to_string(i + 1) + ": Description";
		t->addEntry(e);

		e.original = "%S" + skill.using_message1;
		e.context = "skill.using_message1";
		e.info = "Skill " + std::to_string(i + 1) + ": Using message 1\n%S: Source name";
		t->addEntry(e);

		e.original = "%S" + skill.using_message2;
		e.context = "skill.using_message2";
		e.info = "Skill " + std::to_string(i + 1) + ": Using message 2\n%S: Source name";
		t->addEntry(e);
	}

	for (size_t i = 0; i < Data::items.size(); ++i) {
		const RPG::Item& item = Data::items[i];

		e.original = item.name;
		e.context = "item.name";
		e.info = "Item " + std::to_string(i + 1) + ": Name";
		t->addEntry(e);

		e.original = item.description;
		e.context = "item.description";
		e.info = "Item " + std::to_string(i + 1) + ": Description";
		t->addEntry(e);
	}

	for (size_t i = 0; i < Data::enemies.size(); ++i) {
		const RPG::Enemy& enemy = Data::enemies[i];

		e.original = enemy.name;
		e.context = "enemy.name";
		e.info = "Enemy " + std::to_string(i + 1) + ": Name";
		t->addEntry(e);
	}

	for (size_t i = 0; i < Data::states.size(); ++i) {
		const RPG::State& state = Data::states[i];

		e.original = state.name;
		e.context = "state.name";
		e.info = "State " + std::to_string(i + 1) + ": Name";
		t->addEntry(e);

		e.original = "%S" + state.message_actor;
		e.context = "state.message_actor";
		e.info = "State " + std::to_string(i + 1) + ": Message actor\n%S: Target name";
		t->addEntry(e);

		e.original = "%S" + state.message_enemy;
		e.context = "state.message_enemy";
		e.info = "State " + std::to_string(i + 1) + ": Message enemy\n%S: Target name";
		t->addEntry(e);

		e.original = "%S" + state.message_already;
		e.context = "state.message_already";
		e.info = "State " + std::to_string(i + 1) + ": Message already\n%S: Target name";
		t->addEntry(e);

		e.original = "%S" + state.message_affected;
		e.context = "state.message_affected";
		e.info = "State " + std::to_string(i + 1) + ": Message affected\n%S: Target name";
		t->addEntry(e);

		e.original = "%S" + state.message_recovery;
		e.context = "state.message_recovery";
		e.info = "State " + std::to_string(i + 1) + ": Message recovery\n%S: Target name";
		t->addEntry(e);
	}

	for (size_t i = 0; i < Data::battlecommands.commands.size(); ++i) {
		const RPG::BattleCommand& bcmd = Data::battlecommands.commands[i];

		e.original = bcmd.name;
		e.context = "bcmd.name";
		e.info = "Battle command " + std::to_string(i + 1) + ": Name";
		t->addEntry(e);
	}

	// ToDo: Check for new RPG2k
	e.original = "%S" + Data::terms.encounter;
	e.context = "term";
	e.info = "Term: Battle encounter\n%S: Enemy name";
	t->addEntry(e);

	e.original = Data::terms.special_combat;
	e.context = "term";
	e.info = "Term: Battle surprise attack";
	t->addEntry(e);

	e.original = Data::terms.escape_success;
	e.context = "term";
	e.info = "Term: Battle escape success";
	t->addEntry(e);

	e.original = Data::terms.escape_failure;
	e.context = "term";
	e.info = "Term: Battle escape failed";
	t->addEntry(e);

	e.original = Data::terms.victory;
	e.context = "term";
	e.info = "Term: Battle victory";
	t->addEntry(e);

	e.original = Data::terms.defeat;
	e.context = "term";
	e.info = "Term: Battle defeat";
	t->addEntry(e);

	e.original = "%V %U" + Data::terms.exp_received;
	e.context = "term";
	e.info = "Term: Battle exp received\n%V: EXP amount\n%U: EXP term";
	t->addEntry(e);

	e.original = Data::terms.gold_recieved_a + "%V%U" + Data::terms.gold_recieved_b;
	e.context = "term";
	e.info = "Term: Battle gold received\n%V: Gold amount\n%U: Gold term";
	t->addEntry(e);

	e.original = "%S" + Data::terms.item_recieved;
	e.context = "term";
	e.info = "Term: Battle item received\n%S: Item name";
	t->addEntry(e);

	e.original = "%S" + Data::terms.attacking;
	e.context = "term";
	e.info = "Term: Battle attack\n%S: Source name";
	t->addEntry(e);

	e.original = "%S" + Data::terms.actor_critical;
	e.context = "term";
	e.info = "Term: Battle ally critical hit\n%S: Source name\n%O: Target name";
	t->addEntry(e);

	e.original = "%S" + Data::terms.enemy_critical;
	e.context = "term";
	e.info = "Term: Battle enemy critical hit\n%S: Source name\n%O: Target name";
	t->addEntry(e);

	e.original = "%S" + Data::terms.defending;
	e.context = "term";
	e.info = "Term: Battle defending\n%S: Source name";
	t->addEntry(e);

	e.original = "%S" + Data::terms.observing;
	e.context = "term";
	e.info = "Term: Battle observing\n%S: Source name";
	t->addEntry(e);

	e.original = "%S" + Data::terms.focus;
	e.context = "term";
	e.info = "Term: Battle focus\n%S: Source name";
	t->addEntry(e);

	e.original = "%S" + Data::terms.autodestruction;
	e.context = "term";
	e.info = "Term: Battle autodestruct\n%S: Source name";
	t->addEntry(e);

	e.original = "%S" + Data::terms.enemy_escape;
	e.context = "term";
	e.info = "Term: Battle enemy escape\n%S: Source name";
	t->addEntry(e);

	e.original = "%S" + Data::terms.enemy_transform;
	e.context = "term";
	e.info = "Term: Battle enemy transform\n%S: Source name";
	t->addEntry(e);

	e.original = "%S %V" + Data::terms.enemy_damaged;
	e.context = "term";
	e.info = "Term: Battle enemy damaged\n%S: Source name\n%V: Damage amount";
	t->addEntry(e);

	e.original = "%S" + Data::terms.enemy_undamaged;
	e.context = "term";
	e.info = "Term: Battle enemy undamaged\n%S: Target name";
	t->addEntry(e);

	e.original = "%S %V" + Data::terms.actor_damaged;
	e.context = "term";
	e.info = "Term: Battle actor damaged\n%S: Source name\n%V: Damage amount";
	t->addEntry(e);

	e.original = "%S" + Data::terms.actor_undamaged;
	e.context = "term";
	e.info = "Term: Battle actor undamaged\n%S: Target name";
	t->addEntry(e);

	e.original = "%O" + Data::terms.skill_failure_a;
	e.context = "term";
	e.info = "Term: Battle skill failure a\n%S: Source name\n%O: Target name";
	t->addEntry(e);

	e.original = "%O" + Data::terms.skill_failure_b;
	e.context = "term";
	e.info = "Term: Battle skill failure b\n%S: Source name\n%O: Target name";
	t->addEntry(e);

	e.original = "%O" + Data::terms.skill_failure_c;
	e.context = "term";
	e.info = "Term: Battle skill failure c\n%S: Source name\n%O: Target name";
	t->addEntry(e);

	e.original = "%O" + Data::terms.dodge;
	e.context = "term";
	e.info = "Term: Battle dodge\n%S: Source name\n%O: Target name";
	t->addEntry(e);

	e.original = "%S %O" + Data::terms.use_item;
	e.context = "term";
	e.info = "Term: Battle use item\n%S: Source name\n%O: Item name";
	t->addEntry(e);

	e.original = "%S" + Data::terms.hp_recovery ;
	e.context = "term";
	e.info = "Term: Battle hp recovery\n%S: Source name\n%V: HP amount\n%U: HP term";
	t->addEntry(e);

	e.original = "%S %V %U" + Data::terms.parameter_increase;
	e.context = "term";
	e.info = "Term: Battle parameter increase\n%S: Source name\n%V: Parameter amount\n%U: Parameter term";
	t->addEntry(e);

	e.original = "%S %V %U" + Data::terms.parameter_decrease;
	e.context = "term";
	e.info = "Term: Battle parameter decrease\n%S: Source name\n%V: Parameter amount\n%U: Parameter term";
	t->addEntry(e);

	e.original = "%S %V %U" + Data::terms.actor_hp_absorbed;
	e.context = "term";
	e.info = "Term: Battle actor_hp_absorbed\n%S: Source name\n%O: Target name\n%V: Parameter amount\n%U: Parameter term";
	t->addEntry(e);

	e.original = "%S %V %U" + Data::terms.enemy_hp_absorbed;
	e.context = "term";
	e.info = "Term: Battle enemy_hp_absorbed\n%S: Source name\n%O: Target name\n%V: Parameter amount\n%U: Parameter term";
	t->addEntry(e);

	e.original = "%S %O" + Data::terms.resistance_increase;
	e.context = "term";
	e.info = "Term: Battle resistance_increase\n%S: Source name\n%O: Parameter term";
	t->addEntry(e);

	e.original = "%S %O" + Data::terms.resistance_decrease;
	e.context = "term";
	e.info = "Term: Battle resistance_decrease\n%S: Source name\n%O: Parameter term";
	t->addEntry(e);

	e.original = "%S %V %U" + Data::terms.level_up;
	e.context = "term";
	e.info = "Term: Battle level_up\n%S: Source name\n%U: Level\n%V: Level term";
	t->addEntry(e);

	e.original = "%S" + Data::terms.skill_learned;
	e.context = "term";
	e.info = "Term: Battle skill_learned\n%S: Source name\n%O: Skill name";
	t->addEntry(e);

	e.original = Data::terms.battle_start;
	e.context = "term";
	e.info = "Term: battle_start";
	t->addEntry(e);

	e.original = Data::terms.miss;
	e.context = "term";
	e.info = "Term: miss";
	t->addEntry(e);

	e.original = Data::terms.shop_greeting1;
	e.context = "term";
	e.info = "Term: shop_greeting1";
	t->addEntry(e);

	e.original = Data::terms.shop_regreeting1;
	e.context = "term";
	e.info = "Term: shop_regreeting1";
	t->addEntry(e);

	e.original = Data::terms.shop_buy1;
	e.context = "term";
	e.info = "Term: shop_buy1";
	t->addEntry(e);

	e.original = Data::terms.shop_sell1;
	e.context = "term";
	e.info = "Term: shop_sell1";
	t->addEntry(e);

	e.original = Data::terms.shop_leave1;
	e.context = "term";
	e.info = "Term: shop_leave1";
	t->addEntry(e);

	e.original = Data::terms.shop_buy_select1;
	e.context = "term";
	e.info = "Term: shop_buy_select1";
	t->addEntry(e);

	e.original = Data::terms.shop_buy_number1;
	e.context = "term";
	e.info = "Term: shop_buy_number1";
	t->addEntry(e);

	e.original = Data::terms.shop_purchased1;
	e.context = "term";
	e.info = "Term: shop_purchased1";
	t->addEntry(e);

	e.original = Data::terms.shop_sell_select1;
	e.context = "term";
	e.info = "Term: shop_sell_select1";
	t->addEntry(e);

	e.original = Data::terms.shop_sell_number1;
	e.context = "term";
	e.info = "Term: shop_sell_number1";
	t->addEntry(e);

	e.original = Data::terms.shop_sold1;
	e.context = "term";
	e.info = "Term: shop_sold1";
	t->addEntry(e);

	e.original = Data::terms.shop_greeting2;
	e.context = "term";
	e.info = "Term: shop_greeting2";
	t->addEntry(e);

	e.original = Data::terms.shop_regreeting2;
	e.context = "term";
	e.info = "Term: shop_regreeting2";
	t->addEntry(e);

	e.original = Data::terms.shop_buy2;
	e.context = "term";
	e.info = "Term: shop_buy2";
	t->addEntry(e);

	e.original = Data::terms.shop_sell2;
	e.context = "term";
	e.info = "Term: shop_sell2";
	t->addEntry(e);

	e.original = Data::terms.shop_leave2;
	e.context = "term";
	e.info = "Term: shop_leave2";
	t->addEntry(e);

	e.original = Data::terms.shop_buy_select2;
	e.context = "term";
	e.info = "Term: shop_buy_select2";
	t->addEntry(e);

	e.original = Data::terms.shop_buy_number2;
	e.context = "term";
	e.info = "Term: shop_buy_number2";
	t->addEntry(e);

	e.original = Data::terms.shop_purchased2;
	e.context = "term";
	e.info = "Term: shop_purchased2";
	t->addEntry(e);

	e.original = Data::terms.shop_sell_select2;
	e.context = "term";
	e.info = "Term: shop_sell_select2";
	t->addEntry(e);

	e.original = Data::terms.shop_sell_number2;
	e.context = "term";
	e.info = "Term: shop_sell_number2";
	t->addEntry(e);

	e.original = Data::terms.shop_sold2;
	e.context = "term";
	e.info = "Term: shop_sold2";
	t->addEntry(e);

	e.original = Data::terms.shop_greeting3;
	e.context = "term";
	e.info = "Term: shop_greeting3";
	t->addEntry(e);

	e.original = Data::terms.shop_regreeting3;
	e.context = "term";
	e.info = "Term: shop_regreeting3";
	t->addEntry(e);

	e.original = Data::terms.shop_buy3;
	e.context = "term";
	e.info = "Term: shop_buy3";
	t->addEntry(e);

	e.original = Data::terms.shop_sell3;
	e.context = "term";
	e.info = "Term: shop_sell3";
	t->addEntry(e);

	e.original = Data::terms.shop_leave3;
	e.context = "term";
	e.info = "Term: shop_leave3";
	t->addEntry(e);

	e.original = Data::terms.shop_buy_select3;
	e.context = "term";
	e.info = "Term: shop_buy_select3";
	t->addEntry(e);

	e.original = Data::terms.shop_buy_number3;
	e.context = "term";
	e.info = "Term: shop_buy_number3";
	t->addEntry(e);

	e.original = Data::terms.shop_purchased3;
	e.context = "term";
	e.info = "Term: shop_purchased3";
	t->addEntry(e);

	e.original = Data::terms.shop_sell_select3;
	e.context = "term";
	e.info = "Term: shop_sell_select3";
	t->addEntry(e);

	e.original = Data::terms.shop_sell_number3;
	e.context = "term";
	e.info = "Term: shop_sell_number3";
	t->addEntry(e);

	e.original = Data::terms.shop_sold3;
	e.context = "term";
	e.info = "Term: shop_sold3";
	t->addEntry(e);

	// greeting 1 and 2 concated because it's one sentence
	e.original = Data::terms.inn_a_greeting_1 + "%V%U" + Data::terms.inn_a_greeting_2;
	e.context = "term";
	e.info = "Term: inn_a_greeting_1\n%V: Gold amount\n%U: Gold term";
	t->addEntry(e);

	e.original = Data::terms.inn_a_greeting_3;
	e.context = "term";
	e.info = "Term: inn_a_greeting_2";
	t->addEntry(e);

	e.original = Data::terms.inn_a_accept;
	e.context = "term";
	e.info = "Term: inn_a_accept";
	t->addEntry(e);

	e.original = Data::terms.inn_a_cancel;
	e.context = "term";
	e.info = "Term: inn_a_cancel";
	t->addEntry(e);

	// greeting 1 and 2 concated because it's one sentence
	e.original = Data::terms.inn_b_greeting_1 + "%V%U" + Data::terms.inn_b_greeting_2;
	e.context = "term";
	e.info = "Term: inn_b_greeting_1\n%V: Gold amount\n%U: Gold term";
	t->addEntry(e);

	e.original = Data::terms.inn_b_greeting_3;
	e.context = "term";
	e.info = "Term: inn_b_greeting_2";
	t->addEntry(e);

	e.original = Data::terms.inn_b_accept;
	e.context = "term";
	e.info = "Term: inn_b_accept";
	t->addEntry(e);

	e.original = Data::terms.inn_b_cancel;
	e.context = "term";
	e.info = "Term: inn_b_cancel";
	t->addEntry(e);

	e.original = Data::terms.possessed_items;
	e.context = "term";
	e.info = "Term: possessed_items";
	t->addEntry(e);

	e.original = Data::terms.equipped_items;
	e.context = "term";
	e.info = "Term: equipped_items";
	t->addEntry(e);

	e.original = Data::terms.gold;
	e.context = "term";
	e.info = "Term: gold";
	t->addEntry(e);

	e.original = Data::terms.battle_fight;
	e.context = "term";
	e.info = "Term: battle_fight";
	t->addEntry(e);

	e.original = Data::terms.battle_auto;
	e.context = "term";
	e.info = "Term: battle_auto";
	t->addEntry(e);

	e.original = Data::terms.battle_escape;
	e.context = "term";
	e.info = "Term: battle_escape";
	t->addEntry(e);

	e.original = Data::terms.command_attack;
	e.context = "term";
	e.info = "Term: command_attack";
	t->addEntry(e);

	e.original = Data::terms.command_defend;
	e.context = "term";
	e.info = "Term: command_defend";
	t->addEntry(e);

	e.original = Data::terms.command_item;
	e.context = "term";
	e.info = "Term: command_item";
	t->addEntry(e);

	e.original = Data::terms.command_skill;
	e.context = "term";
	e.info = "Term: command_skill";
	t->addEntry(e);

	e.original = Data::terms.menu_equipment;
	e.context = "term";
	e.info = "Term: menu_equipment";
	t->addEntry(e);

	e.original = Data::terms.menu_save;
	e.context = "term";
	e.info = "Term: menu_save";
	t->addEntry(e);

	e.original = Data::terms.menu_quit;
	e.context = "term";
	e.info = "Term: menu_quit";
	t->addEntry(e);

	e.original = Data::terms.new_game;
	e.context = "term";
	e.info = "Term: new_game";
	t->addEntry(e);

	e.original = Data::terms.load_game;
	e.context = "term";
	e.info = "Term: load_game";
	t->addEntry(e);

	e.original = Data::terms.exit_game;
	e.context = "term";
	e.info = "Term: exit_game";
	t->addEntry(e);

	e.original = Data::terms.status;
	e.context = "term";
	e.info = "Term: status";
	t->addEntry(e);

	e.original = Data::terms.row;
	e.context = "term";
	e.info = "Term: row";
	t->addEntry(e);

	e.original = Data::terms.order;
	e.context = "term";
	e.info = "Term: order";
	t->addEntry(e);

	e.original = Data::terms.wait_on;
	e.context = "term";
	e.info = "Term: wait_on";
	t->addEntry(e);

	e.original = Data::terms.wait_off;
	e.context = "term";
	e.info = "Term: wait_off";
	t->addEntry(e);

	e.original = Data::terms.level;
	e.context = "term";
	e.info = "Term: level";
	t->addEntry(e);

	e.original = Data::terms.health_points;
	e.context = "term";
	e.info = "Term: health_points";
	t->addEntry(e);

	e.original = Data::terms.spirit_points;
	e.context = "term";
	e.info = "Term: spirit_points";
	t->addEntry(e);

	e.original = Data::terms.normal_status;
	e.context = "term";
	e.info = "Term: normal_status";
	t->addEntry(e);

	e.original = Data::terms.exp_short;
	e.context = "term";
	e.info = "Term: exp_short";
	t->addEntry(e);

	e.original = Data::terms.lvl_short;
	e.context = "term";
	e.info = "Term: lvl_short";
	t->addEntry(e);

	e.original = Data::terms.hp_short;
	e.context = "term";
	e.info = "Term: hp_short";
	t->addEntry(e);

	e.original = Data::terms.sp_short;
	e.context = "term";
	e.info = "Term: sp_short";
	t->addEntry(e);

	e.original = Data::terms.sp_cost;
	e.context = "term";
	e.info = "Term: sp_cost";
	t->addEntry(e);

	e.original = Data::terms.attack;
	e.context = "term";
	e.info = "Term: attack";
	t->addEntry(e);

	e.original = Data::terms.defense;
	e.context = "term";
	e.info = "Term: defense";
	t->addEntry(e);

	e.original = Data::terms.spirit;
	e.context = "term";
	e.info = "Term: spirit";
	t->addEntry(e);

	e.original = Data::terms.agility;
	e.context = "term";
	e.info = "Term: agility";
	t->addEntry(e);

	e.original = Data::terms.weapon;
	e.context = "term";
	e.info = "Term: weapon";
	t->addEntry(e);

	e.original = Data::terms.shield;
	e.context = "term";
	e.info = "Term: shield";
	t->addEntry(e);

	e.original = Data::terms.armor;
	e.context = "term";
	e.info = "Term: armor";
	t->addEntry(e);

	e.original = Data::terms.helmet;
	e.context = "term";
	e.info = "Term: helmet";
	t->addEntry(e);

	e.original = Data::terms.accessory;
	e.context = "term";
	e.info = "Term: accessory";
	t->addEntry(e);

	e.original = Data::terms.save_game_message;
	e.context = "term";
	e.info = "Term: save_game_message";
	t->addEntry(e);

	e.original = Data::terms.load_game_message;
	e.context = "term";
	e.info = "Term: load_game_message";
	t->addEntry(e);

	e.original = Data::terms.file;
	e.context = "term";
	e.info = "Term: file";
	t->addEntry(e);

	e.original = Data::terms.exit_game_message;
	e.context = "term";
	e.info = "Term: exit_game_message";
	t->addEntry(e);

	e.original = Data::terms.yes;
	e.context = "term";
	e.info = "Term: yes";
	t->addEntry(e);

	e.original = Data::terms.no;
	e.context = "term";
	e.info = "Term: no";
	t->addEntry(e);

	// Read common events
	bool has_message = false;
	std::vector<std::string> choices;

	for (size_t i = 0; i < Data::commonevents.size(); ++i) {
		int evt_count = i + 1;

		const std::vector<RPG::EventCommand>& events = Data::commonevents[i].event_commands;
		for (size_t j = 0; j < events.size(); ++j) {
			const RPG::EventCommand& evt = events[j];
			int line_count = j + 1;

			e.info = "Common Event " + std::to_string(evt_count) + ", Line " + std::to_string(line_count);
			e.context = "event";

			switch (evt.code) {
			case Cmd::ShowMessage:
				if (has_message) {
					// New message
					if (!e.original.empty()) {
						e.original.pop_back();
						t->addEntry(e);
					}
				}

				has_message = true;
				e.original = evt.string + "\n";

				break;
			case Cmd::ShowMessage_2:
				if (!has_message) {
					// shouldn't happen
					e.original = "";
				}

				// Next message line
				e.original += evt.string + "\n";
				break;
			case Cmd::ShowChoice:
				if (has_message) {
					has_message = false;
					if (!e.original.empty()) {
						e.original.pop_back();
						t->addEntry(e);
					}
				}

				e.info += " (Choice)";

				choices.clear();
				getStrings(choices, events, j);
				if (!choices.empty()) {
					e.original = choices[0] + "\n";

					for (size_t choice_i = 1; choice_i < choices.size(); ++choice_i) {
						e.original += choices[choice_i] + "\n";
					}

					if (!e.original.empty()) {
						e.original.pop_back();
						t->addEntry(e);
					}
				}

				break;
			default:
				if (has_message) {
					has_message = false;
					if (!e.original.empty()) {
						e.original.pop_back();
						t->addEntry(e);
					}
				}
				break;
			}
		}
	}

	return t;
}

Translation* Translation::fromLMU(const std::string& filename, const std::string& encoding) {
	RPG::Map map = *LMU_Reader::Load(filename, encoding);

	bool has_message = false;
	std::vector<std::string> choices;

	Translation* t = new Translation();

	for (size_t i = 0; i < map.events.size(); ++i) {
		const RPG::Event rpg_evt = map.events[i];

		for (size_t j = 0; j < rpg_evt.pages.size(); ++j) {
			int page_count = j + 1;
			const RPG::EventPage& page = rpg_evt.pages[j];
			Entry e;

			for (size_t k = 0; k < page.event_commands.size(); ++k) {
				const RPG::EventCommand& evt = page.event_commands[k];
				int line_count = k + 1;

				e.info = "Event " + std::to_string(rpg_evt.ID) + ", Page " + std::to_string(page_count) + ", Line " + std::to_string(line_count);
				e.context = "event" + std::to_string(rpg_evt.ID);

				switch (evt.code) {
				case Cmd::ShowMessage:
					if (has_message) {
						// New message
						if (!e.original.empty()) {
							e.original.pop_back();
							t->addEntry(e);
						}
					}

					has_message = true;
					e.original = evt.string + "\n";

					break;
				case Cmd::ShowMessage_2:
					if (!has_message) {
						// shouldn't happen
						e.original = "";
					}

					// Next message line
					e.original += evt.string + "\n";
					break;
				case Cmd::ShowChoice:
					if (has_message) {
						has_message = false;
						if (!e.original.empty()) {
							e.original.pop_back();
							t->addEntry(e);
						}
					}

					e.info += " (Choice)";

					choices.clear();
					getStrings(choices, page.event_commands, j);
					if (!choices.empty()) {
						e.original = choices[0] + "\n";

						for (size_t choice_i = 1; choice_i < choices.size(); ++choice_i) {
							e.original += choices[choice_i] + "\n";
						}

						if (!e.original.empty()) {
							e.original.pop_back();
							t->addEntry(e);
						}
					}

					break;
				default:
					if (has_message) {
						has_message = false;
						if (!e.original.empty()) {
							e.original.pop_back();
							t->addEntry(e);
						}
					}
					break;
				}
			}
		}
	}

	return t;
}

Translation* Translation::fromPO(const std::string& filename) {
	return nullptr;
}

static void write_n(std::ostream& out, std::string line, std::string prefix) {
	if (line.find("\n") != std::string::npos) {
		std::stringstream ss(escape(line));
		out << prefix << " \"\"" << std::endl;

		std::string item;
		bool write_n = false;
		while (std::getline(ss, item, '\n')) {
			if (write_n) {
				out << "\\n\"" << std::endl;
			}

			write_n = true;
			if (item.length() > 0) {
				out << "\"" << item;
			}
		}
		out << "\"" << std::endl;
	} else {
		out << prefix << " \"" << escape(line) << "\"" << std::endl;
	}
}

void Translation::Entry::write(std::ostream& out) const {
	if (!context.empty()) {
		out << "msgctxt \"" << context << "\"" << std::endl;
	}

	write_n(out, original, "msgid");
	write_n(out, translation, "msgstr");
}
