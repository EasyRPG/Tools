/*
 * Copyright (c) 2020 LcfViz authors
 * This file is released under the MIT License
 * http://opensource.org/licenses/MIT
 */

#include "utils.h"

#include <algorithm>

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
