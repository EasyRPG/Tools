/* main.h
   Copyright (C) 2024 EasyRPG Project <https://github.com/EasyRPG/>.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef MAIN_H
#define MAIN_H

// Headers
#include <string>

// Types
using ErrorCallbackParam = void *;
using ErrorCallbackFunc = void (*) (const std::string&, ErrorCallbackParam);

using L2IConfig = struct l2i_config_t {
	std::string database;
	std::string chipset;
	std::string encoding;
	std::string map;
	bool verbose;
	bool no_background;
	bool no_lowertiles;
	bool no_uppertiles;
	bool no_events;
	bool ignore_conditions;
	bool simulate_movement;
};

#ifdef WITH_GUI
unsigned char * makeImage(L2IConfig conf, int &w, int &h, ErrorCallbackFunc error_cb,
	ErrorCallbackParam param = nullptr);
#endif

#endif
