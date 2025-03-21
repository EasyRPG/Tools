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
#include <lcf/span.h>
#include <string_view>

namespace Utils {
	std::string GetFilename(const std::string& str);
	bool HasExt(const std::string& path, const std::string& ext);
	std::string Join(const std::vector<std::string>& lines, char join_char = '\n');
	std::vector<std::string> Split(const std::string& line, char split_char = '\n');
	std::string LowerCase(const std::string &in);
	std::string RemoveControlChars(std::string_view s);
	bool ReadLine(std::istream& is, std::string& line_out);
	std::string_view TrimWhitespace(std::string_view s);

	std::vector<std::string> GetChoices(lcf::Span<lcf::rpg::EventCommand> list, int start_index);

	std::string Escape(const std::string& str);
}

#endif
