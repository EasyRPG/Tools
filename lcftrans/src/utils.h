/*
 * Copyright (c) 2020 LcfTrans authors
 * This file is released under the MIT License
 * http://opensource.org/licenses/MIT
 */

#ifndef LCFTRANS_UTILS
#define LCFTRANS_UTILS

#include <string>
#include <vector>
#include <lcf/rpg/eventcommand.h>

namespace Utils {
	std::string GetFilename(const std::string& str);
	bool HasExt(const std::string& path, const std::string& ext);
	std::string Join(const std::vector<std::string>& lines, char join_char = '\n');
	std::string LowerCase(const std::string &in);
	std::string RemoveControlChars(lcf::StringView s);

	std::vector<std::string> GetChoices(const std::vector<lcf::rpg::EventCommand>& list, int start_index);

	std::string Escape(const std::string& str);
}

#endif

