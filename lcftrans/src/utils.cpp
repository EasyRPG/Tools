/*
 * Copyright (c) 2020 LcfTrans authors
 * This file is released under the MIT License
 * http://opensource.org/licenses/MIT
 */

#include "utils.h"

#include <algorithm>
#include <sstream>

std::string Utils::GetFilename(const std::string& str) {
	std::string s = str;
#ifdef _WIN32
	std::replace(s.begin(), s.end(), '\\', '/');
#endif
	// Extension
	size_t found = s.find_last_of('.');
	if (found != std::string::npos)
	{
		s = s.substr(0, found);
	}

	// Filename
	found = s.find_last_of('/');
	if (found == std::string::npos)	{
		return s;
	}

	s = s.substr(found + 1);
	return s;
}

bool Utils::HasExt(const std::string& path, const std::string& ext) {
	if (path.size() < ext.size()) {
		return false;
	}
	for (size_t i = 0; i < ext.size(); ++i) {
		if (std::tolower(path[path.size()-i-1]) != ext[ext.size()-i-1]) {
			return false;
		}
	}
	return true;
}

std::string Utils::Join(const std::vector<std::string>& lines, char join_char) {
	if (lines.empty()) {
		return "";
	}

	std::string ret;
	for (const std::string& s : lines) {
		ret += s + join_char;
	}
	ret.pop_back(); // remove last join_char
	return ret;
}

std::vector<std::string> Utils::Split(const std::string& line, char split_char) {
	std::vector<std::string> tokens;
	std::string cur_token;

	for (auto c: line) {
		if (c == split_char) {
			tokens.push_back(cur_token);
			cur_token.clear();
			continue;
		}

		cur_token.push_back(c);
	}

	tokens.push_back(cur_token);

	return tokens;
}

std::string Utils::LowerCase(const std::string& in) {
	auto lower = [](char& c) -> char {
		if (c >= 'A' && c <= 'Z') {
			return c + 'a' - 'A';
		} else {
			return c;
		}
	};

	std::string data = in;
	std::transform(data.begin(), data.end(), data.begin(), lower);
	return data;
}

std::string Utils::RemoveControlChars(std::string_view s) {
	// RPG_RT ignores any control characters within messages.
	std::string out = lcf::ToString(s);
	auto iter = std::remove_if(out.begin(), out.end(), [](const char ch) { return (ch >= 0x0 && ch <= 0x1F) || ch == 0x7F; });
	out.erase(iter, out.end());
	return out;
}

// based on https://stackoverflow.com/questions/6089231/
bool Utils::ReadLine(std::istream& is, std::string& line_out) {
	std::istream::sentry se(is, true);
	std::streambuf* sb = is.rdbuf();

	if (!is) {
		return false;
	}

	line_out.clear();

	for(;;) {
		int c = sb->sbumpc();
		switch (c) {
			case '\n':
				return true;
			case '\r':
				if (sb->sgetc() == '\n') {
					sb->sbumpc();
				}
				return true;
			case EOF:
				// Also handle the case when the last line has no line ending
				if (line_out.empty()) {
					is.setstate(std::ios::eofbit);
					return false;
				}
				return true;
			default:
				line_out += (char)c;
		}
	}
}

std::string_view Utils::TrimWhitespace(std::string_view s) {
	size_t left = 0;
	for (auto& c: s) {
		if (std::isspace(static_cast<int>(c))) {
			++left;
		} else {
			break;
		}
	}
	s.remove_prefix(left);

	size_t right = 0;
	for (auto it = s.crbegin(); it != s.crend(); ++it) {
		if (std::isspace(static_cast<int>(*it))) {
			++right;
		} else {
			break;
		}
	}
	s.remove_suffix(right);

	return s;
}

std::vector<std::string> Utils::GetChoices(lcf::Span<lcf::rpg::EventCommand> list, int start_index) {
	using Cmd = lcf::rpg::EventCommand::Code;
	constexpr int max_num_choices = 4;

	int current_indent = list[start_index].indent;
	std::vector<std::string> s_choices;
	for (int index_temp = start_index; index_temp < static_cast<int>(list.size()); ++index_temp) {
		const auto& com = list[index_temp];
		if (com.indent != current_indent) {
			continue;
		}

		if (static_cast<Cmd>(com.code) == Cmd::ShowChoiceOption && com.parameters.size() > 0 && com.parameters[0] < max_num_choices) {
			// Choice found
			s_choices.push_back(Utils::RemoveControlChars(list[index_temp].string));
		}

		if (static_cast<Cmd>(com.code) == Cmd::ShowChoiceEnd) {
			break;
		}
	}
	return s_choices;
}

std::string Utils::Escape(const std::string& str) {
	std::stringstream s;
	for (char c : str) {
		switch (c) {
			case '"':
				s << "\\\"";
				break;
			case '\\':
				s << "\\\\";
				break;
			default:
				s << c;
		}
	}

	return s.str();
}
