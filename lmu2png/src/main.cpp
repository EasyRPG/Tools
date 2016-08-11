/* main.cpp, lmu2png main file.
   Copyright (C) 2016 EasyRPG Project <https://github.com/EasyRPG/>.

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

// Headers
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#ifndef _WIN32
#  include <unistd.h>
#endif
#include <SDL_image.h>
#include <ldb_reader.h>
#include <lmu_reader.h>
#include <reader_lcf.h>
#include <rpg_map.h>
#include <data.h>
#include "chipset.h"

// prevent SDL main rename
#undef main

const std::string usage = R"(Usage: lmu2png map.lmu [options]
Options:
	-d database.ldb
	-c chipset.png
	-e encoding
	-o output.png
	--no-background
	--no-lowertiles
	--no-uppertiles
	--no-events
)";

std::string GetFileDirectory (const std::string& file) {
  size_t found = file.find_last_of("/\\");
  return found == std::string::npos ? "./" : file.substr(0,found + 1);
}

bool Exists(const std::string& filename) {
#ifdef _WIN32
	#define access _access
#endif
	return access(filename.c_str(), 0) != -1;
}

std::string path;

std::string FindResource(const std::string& folder, const std::string& base_name) {
	static const std::string rtp2k(getenv("RPG2K_RTP_PATH"));
	static const std::string rtp2k3(getenv("RPG2K3_RTP_PATH"));
	for (const auto& dir : {path, rtp2k, rtp2k3}) {
		for (const auto& ext : {".png", ".bmp", ".xyz"}) {
			if (Exists(dir + "/" + folder + "/" + base_name + ext))
				return dir + "/" + folder + "/" + base_name + ext;
		}
	}
	return "";
}

SDL_Surface* LoadImage(const char* image_path, bool transparent = false) {
	SDL_Surface* image = IMG_Load(image_path);
	if (!image) {
		std::cout << IMG_GetError() << std::endl;
		exit(EXIT_FAILURE);
	}

	if (transparent && image->format->palette) {
		// Set as color key the first color in the palette
		SDL_SetColorKey(image, SDL_TRUE, 0);
	}

	return image;
}

int main(int argc, char** argv) {
	std::string database;
	std::string chipset;
	std::string encoding;
	std::string output;
	std::string input;
	bool show_background = true;
	bool show_lowertiles = true;
	bool show_uppertiles = true;
	bool show_events = true;

	// Parse arguments
	for (int i = 1; i < argc; ++i) {
		std::string arg (argv[i]);
		if (arg == "-h" || arg == "--help") {
			std::cout << usage << std::endl;
			exit(EXIT_SUCCESS);
		} else if (arg == "-d") {
			if (++i < argc)
				database = argv[i];
		} else if (arg == "-c") {
			if (++i < argc)
				chipset = argv[i];
		} else if (arg == "-e") {
			if (++i < argc)
				encoding = argv[i];
		} else if (arg == "-o") {
			if (++i < argc)
				output = argv[i];
		} else if (arg == "--no-background") {
			show_background = false;
		} else if (arg == "--no-lowertiles") {
			show_lowertiles = false;
		} else if (arg == "--no-uppertiles") {
			show_uppertiles = false;
		} else if (arg == "--no-events") {
			show_events = false;
		} else {
			input = arg;
		}
	}

	if (input.empty()) {
		std::cout << usage << std::endl;
		exit(EXIT_FAILURE);
	}

	if (!Exists(input)) {
		std::cout << "Input map file " << input << " can't be found." << std::endl;
		exit(EXIT_FAILURE);
	}

	path = GetFileDirectory(input);

	if (encoding.empty())
		encoding = ReaderUtil::GetEncoding(path + "RPG_RT.ini");

	std::unique_ptr<RPG::Map> map(LMU_Reader::Load(input, encoding));
	if (!map) {
		std::cout << LcfReader::GetError() << std::endl;
		exit(EXIT_FAILURE);
	}

	if (output.empty()) {
		output = input.substr(0, input.length() - 3) + "png";
	}

	if (chipset.empty()) {
		// Get chipset from database
		if (database.empty())
			database = path + "RPG_RT.ldb";

		if (!LDB_Reader::Load(database, encoding)) {
			std::cout << LcfReader::GetError() << std::endl;
			exit(EXIT_FAILURE);
		}

		assert(map->chipset_id <= Data::chipsets.size());
		std::string chipset_base(Data::chipsets[map->chipset_id - 1].chipset_name);

		chipset = FindResource("ChipSet", chipset_base);
		if (chipset.empty()) {
			std::cout << "Chipset " << chipset_base << " can't be found." << std::endl;
			exit(EXIT_FAILURE);
		}
	}

	if (chipset.substr(chipset.length() - 3) == "xyz") {
		std::cout << "Can't open chipset " << chipset << ". XYZ format is not supported." << std::endl;
		exit(EXIT_FAILURE);
	}

	SDL_Surface* chipset_img = LoadImage(chipset.c_str(), true);

	SDL_Surface* output_img = SDL_CreateRGBSurface(0, map->width * 16, map->height * 16, 32, 0, 0, 0, 0);

	// Draw parallax background
	if (show_background && !map->parallax_name.empty()) {
		std::string background(FindResource("Panorama", map->parallax_name));
		if (background.empty()) {
			std::cout << "Can't find parallax background " << map->parallax_name << std::endl;
		} else if (background.substr(background.length() - 3) == "xyz") {
			std::cout << "Can't open parallax background " << background << ". XYZ format is not supported." << std::endl;
			exit(EXIT_FAILURE);
		} else {
			SDL_Surface* background_img(LoadImage(background.c_str()));
			SDL_Rect dst_rect = background_img->clip_rect;
			// Fill screen with copies of the background
			for (dst_rect.x = 0; dst_rect.x < output_img->w; dst_rect.x += background_img->w) {
				for (dst_rect.y = 0; dst_rect.y < output_img->h; dst_rect.y += background_img->h) {
					SDL_BlitSurface(background_img, nullptr, output_img, &dst_rect);
				}
			}
		}
	}

	// Draw lower and upper tiles
	stChipset gen;
	gen.GenerateFromSurface(chipset_img);
	for (int y = 0; y < map->height; ++y) {
		for (int x = 0; x < map->width; ++x) {
			if (show_lowertiles)
				gen.RenderTile(output_img, x*16, y*16, map->lower_layer[x+y*map->width], 0);
			if (show_uppertiles)
				gen.RenderTile(output_img, x*16, y*16, map->upper_layer[x+y*map->width], 0);
		}
	}

	// Draw events
	if (show_events) {
		for (const RPG::Event& ev : map->events) {
			const RPG::EventPage* evp = nullptr;
			// Find highest page without conditions
			for (int i = 0; i < ev.pages.size(); ++i) {
				const auto& flg = ev.pages[i].condition.flags;
				if (flg.switch_a || flg.switch_b || flg.variable || flg.item || flg.actor || flg.timer || flg.timer2)
					continue;
				evp = &ev.pages[i];
			}
			if (!evp)
				continue;

			if (evp->character_name.empty())
				gen.RenderTile(output_img, (ev.x)*16, (ev.y)*16, 0x2710 + evp->character_index, 0);
			else {
				std::string charset(FindResource("CharSet", evp->character_name));
				if (charset.empty()) {
					std::cout << "Can't find charset " << evp->character_name << std::endl;
					continue;
				}
				if (charset.substr(charset.length() - 3) == "xyz") {
					std::cout << "Can't open chipset " << charset << ". XYZ format is not supported." << std::endl;
					continue;
				}

				SDL_Surface* charset_img(LoadImage(charset.c_str(), true));
				SDL_Rect src_rect {(evp->character_index % 4) * 72 + evp->character_pattern * 24,
					 (evp->character_index / 4) * 128 + evp->character_direction * 32, 24, 32};
				SDL_Rect dst_rect {ev.x * 16 - 4, ev.y * 16 - 16, 16, 32}; // Why -4 and -16?
				SDL_BlitSurface(charset_img, &src_rect, output_img, &dst_rect);
			}
		}
	}

	if (IMG_SavePNG(output_img, output.c_str()) < 0) {
		std::cout << IMG_GetError() << std::endl;
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
