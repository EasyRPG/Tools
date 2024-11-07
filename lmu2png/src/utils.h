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
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <lcf/rpg/map.h>
#include <FreeImage.h>
#include <memory>
#include "main.h"

// forward declarations

struct Chipset;

// type and other definitions

enum class LAYER : int {
	LOWER = 0,
	UPPER,
	EVENTS
};

struct FIBITMAPDeleter {
	void operator()(FIBITMAP* dib) {
		FreeImage_Unload(dib);
	}
};
using BitmapPtr = std::unique_ptr<FIBITMAP, FIBITMAPDeleter>;

std::string GetFileDirectory(const std::string& file);

bool Exists(const std::string& filename);

void CollectResourcePaths(std::string& main_path);

std::string FindResource(const std::string& folder, const std::string& base_name);

void CustomAlphaCombine(FIBITMAP *src, int sLeft, int sTop, FIBITMAP *dst, int dLeft,
	int dTop, int width, int height);

FIBITMAP* LoadImage(const char* image_path, bool transparent = false);

void DrawTiles(FIBITMAP* output_img, Chipset * gen, uint8_t * csflag,
	std::unique_ptr<lcf::rpg::Map> & map, L2IConfig conf, LAYER flaglayer);

void DrawEvents(FIBITMAP* output_img, Chipset * gen,
	std::unique_ptr<lcf::rpg::Map> & map, LAYER layer);

void RenderCore(FIBITMAP* output_img, uint8_t * csflag,
	std::unique_ptr<lcf::rpg::Map> & map, L2IConfig conf);

#endif
