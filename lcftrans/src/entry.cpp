/*
 * Copyright (c) 2020 LcfTrans authors
 * This file is released under the MIT License
 * http://opensource.org/licenses/MIT
 */

#include "entry.h"
#include "utils.h"

#include <ostream>
#include <sstream>

static void write_n(std::ostream& out, const std::string& line, const std::string& prefix) {
	if (line.find("\n") != std::string::npos) {
		std::stringstream ss(Utils::Escape(line));
		out << prefix << " \"\"" << std::endl;

		std::string item;
		bool write_n = false;
		while (std::getline(ss, item, '\n')) {
			if (write_n) {
				out << "\\n\"" << std::endl;
			}

			out << "\"" << item;

			write_n = true;
		}
		out << "\"" << std::endl;
	} else {
		out << prefix << " \"" << Utils::Escape(line) << "\"" << std::endl;
	}
}

void Entry::write(std::ostream& out) const {
	if (!context.empty()) {
		out << "msgctxt \"" << context << "\"" << std::endl;
	}

	write_n(out, original, "msgid");
	write_n(out, translation, "msgstr");
}
