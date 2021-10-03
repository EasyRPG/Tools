/*
 * Copyright (c) 2020 LcfTrans authors
 * This file is released under the MIT License
 * http://opensource.org/licenses/MIT
 */

#include "entry.h"
#include "utils.h"

#include <ostream>
#include <sstream>

static void write_n(std::ostream& out, const std::vector<std::string>& lines, const std::string& prefix) {
	std::string first_line;
	if (!lines.empty()) {
		first_line = Utils::Escape(lines[0]);
	}

	if (lines.size() <= 1) {
		out << prefix << " \"" << first_line << "\"\n";
	} else {
		std::stringstream ss(first_line);
		out << prefix << " \"\"\n";

		bool write_n = false;
		for (const auto& line: lines) {
			if (write_n) {
				out << "\\n\"\n";
			}

			out << "\"" <<  Utils::Escape(line);
			write_n = true;
		}
		out << "\"\n";
	}
}

void Entry::write(std::ostream& out) const {
	if (!context.empty()) {
		out << "msgctxt \"" << context << "\"" << std::endl;
	}

	write_n(out, original, "msgid");
	write_n(out, translation, "msgstr");
}

bool Entry::hasTranslation() const {
	return !translation.empty() && !(translation.size() == 1 && translation[0].empty());
}
