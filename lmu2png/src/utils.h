/* utils.h
   Copyright (C) 2023 EasyRPG Project <https://github.com/EasyRPG/>.

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

#ifndef UTILS_H
#define UTILS_H

// Headers
#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <lcf/ldb/reader.h>
#include <lcf/lmu/reader.h>
#include <lcf/reader_lcf.h>
#include <lcf/rpg/map.h>
#include <lcf/rpg/chipset.h>
#include "chipset.h"
#include "main.h"

std::string GetFileDirectory(const std::string& file);

bool Exists(const std::string& filename);

void CollectResourcePaths(std::string& main_path);

std::string FindResource(const std::string& folder, const std::string& base_name);

SDL_Surface* LoadImage(const char* image_path, bool transparent = false);

void DrawTiles(SDL_Surface* output_img, stChipset * gen, uint8_t * csflag,
	std::unique_ptr<lcf::rpg::Map> & map, ImgConfig conf, int flaglayer);

void DrawEvents(SDL_Surface* output_img, stChipset * gen,
	std::unique_ptr<lcf::rpg::Map> & map, int layer);

void RenderCore(SDL_Surface* output_img, std::string chipset, uint8_t * csflag,
	std::unique_ptr<lcf::rpg::Map> & map, ImgConfig conf);

#endif
