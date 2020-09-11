/*
 * Copyright (c) 2020 LcfTrans authors
 * This file is released under the MIT License
 * http://opensource.org/licenses/MIT
 */

#ifndef LCFTRANS_ENTRY
#define LCFTRANS_ENTRY

#include <string>

class Entry {
public:
	std::string original; // msgid
	std::string translation; // msgstr
	std::string context; // msgctxt
	std::string info; // #.
	std::string location; // #: // Unused, maybe useful later

	void write(std::ostream& out) const;
};

#endif

