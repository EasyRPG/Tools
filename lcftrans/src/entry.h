/*
 * Copyright (c) 2020 LcfTrans authors
 * This file is released under the MIT License
 * http://opensource.org/licenses/MIT
 */

#ifndef LCFTRANS_ENTRY
#define LCFTRANS_ENTRY

#include <string>
#include <vector>

class Entry {
public:
	std::vector<std::string> original; // msgid
	std::vector<std::string> translation; // msgstr
	std::string context; // msgctxt
	std::vector<std::string> info; // #.
	std::string location; // #: // Unused, maybe useful later
	bool fuzzy = false; // When true write a "#, fuzzy" marker

	void write(std::ostream& out) const;

	bool hasTranslation() const;
};

#endif

