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
#include <fstream>
#include <string>
#include <algorithm>
#include <SDL_image.h>
#include <ldb_reader.h>
#include <lmu_reader.h>
#include <reader_lcf.h>
#include <rpg_map.h>
#include <data.h>
#include "chipset.h"
#include "sdlxyz.h"

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
    std::ifstream infile(filename.c_str());
    return infile.good();
}

std::string path;

std::string FindResource(const std::string& folder, const std::string& base_name) {
	static const std::vector<std::string> dirs = [] {
		char* rtp2k_ptr = getenv("RPG2K_RTP_PATH");
		char* rtp2k3_ptr = getenv("RPG2K3_RTP_PATH");
		std::vector<std::string> dirs = {path};
		if (rtp2k_ptr)
			dirs.emplace_back(rtp2k_ptr);
		if (rtp2k3_ptr)
			dirs.emplace_back(rtp2k3_ptr);
		return dirs;
	}();

	for (const auto& dir : dirs) {
		for (const auto& ext : {".png", ".bmp", ".xyz"}) {
			if (Exists(dir + "/" + folder + "/" + base_name + ext))
				return dir + "/" + folder + "/" + base_name + ext;
		}
	}
	return "";
}

SDL_Surface* LoadImage(const char* image_path, bool transparent = false) {
	// Try XYZ, then IMG_Load
	SDL_Surface* image = LoadImageXYZ(image_path);
	if (!image) {
		image = IMG_Load(image_path);
	}
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

void DrawTiles(SDL_Surface* output_img, stChipset * gen, uint8_t * csflag, std::unique_ptr<RPG::Map> & map, int show_lowertiles, int show_uppertiles, int flaglayer) {
	for (int y = 0; y < map->height; ++y) {
		for (int x = 0; x < map->width; ++x) {
			// Different logic between these.
			int tindex = x + y * map->width;
			if (show_lowertiles) {
				uint16_t tid = map->lower_layer[tindex];
				int l = (csflag[tid] & 0x30) ? 1 : 0;
				if (l == flaglayer)
					gen->RenderTile(output_img, x*16, y*16, map->lower_layer[x+y*map->width], 0);
			}
			if (show_uppertiles) {
				uint16_t tid = map->upper_layer[tindex];
				int l = (csflag[tid] & 0x10) ? 1 : 0;
				if (l == flaglayer)
					gen->RenderTile(output_img, x*16, y*16, map->upper_layer[x+y*map->width], 0);
			}
		}
	}
}

void DrawEvents(SDL_Surface* output_img, stChipset * gen, std::unique_ptr<RPG::Map> & map, int layer) {
	for (const RPG::Event& ev : map->events) {
		const RPG::EventPage* evp = nullptr;
		// Find highest page without conditions
		for (int i = 0; i < (int)ev.pages.size(); ++i) {
			const auto& flg = ev.pages[i].condition.flags;
			if (flg.switch_a || flg.switch_b || flg.variable || flg.item || flg.actor || flg.timer || flg.timer2)
				continue;
			evp = &ev.pages[i];
		}
		if (!evp)
			continue;
		// Event layering
		if (evp->layer >= 0 && evp->layer < 3 && evp->layer != layer)
			continue;

		if (evp->character_name.empty())
			gen->RenderTile(output_img, (ev.x)*16, (ev.y)*16, 0x2710 + evp->character_index, 0);
		else {
			std::string charset(FindResource("CharSet", evp->character_name));
			if (charset.empty()) {
				std::cout << "Can't find charset " << evp->character_name << std::endl;
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

void RenderCore(SDL_Surface* output_img, std::string chipset, uint8_t * csflag, std::unique_ptr<RPG::Map> & map, int show_background, int show_lowertiles, int show_uppertiles, int show_events) {
	SDL_Surface* chipset_img = LoadImage(chipset.c_str(), true);

	stChipset gen;
	gen.GenerateFromSurface(chipset_img);

	// Draw parallax background
	if (show_background && !map->parallax_name.empty()) {
		std::string background(FindResource("Panorama", map->parallax_name));
		if (background.empty()) {
			std::cout << "Can't find parallax background " << map->parallax_name << std::endl;
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

	// Draw below tile layer
	if (show_lowertiles || show_uppertiles)
		DrawTiles(output_img, &gen, csflag, map, show_lowertiles, show_uppertiles, 0);
	// Draw below-player & player-level events
	if (show_events) {
		DrawEvents(output_img, &gen, map, 0);
		DrawEvents(output_img, &gen, map, 1);
	}
	// Draw above tile layer
	if (show_lowertiles || show_uppertiles)
		DrawTiles(output_img, &gen, csflag, map, show_lowertiles, show_uppertiles, 1);
	// Draw events
	if (show_events)
		DrawEvents(output_img, &gen, map, 2);
}

bool MapEventYSort(const RPG::Event& ev1, const RPG::Event& ev2) {
	return ev1.y < ev2.y;
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
	// ChipSet flags
	uint8_t csflag[65536];
	memset(csflag, 0, 65536);

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

		assert(map->chipset_id <= (int)Data::chipsets.size());
		RPG::Chipset & cs = Data::chipsets[map->chipset_id - 1];
		std::string chipset_base(cs.chipset_name);

		chipset = FindResource("ChipSet", chipset_base);
		if (chipset.empty()) {
			std::cout << "Chipset " << chipset_base << " can't be found." << std::endl;
			exit(EXIT_FAILURE);
		}
		// Load flags.
		memset(csflag + 0, cs.passable_data_lower[0], 1000);
		memset(csflag + 1000, cs.passable_data_lower[1], 1000);
		memset(csflag + 2000, cs.passable_data_lower[2], 1000);
		memset(csflag + 3000, cs.passable_data_lower[3], 50);
		memset(csflag + 3050, cs.passable_data_lower[4], 50);
		memset(csflag + 3100, cs.passable_data_lower[5], 900);

		for (int i = 0; i < 12; i++)
			memset(csflag + 4000 + (i * 50), cs.passable_data_lower[6 + i], 50);

		for (int i = 0; i < 144; i++) {
			csflag[5000 + i] = cs.passable_data_lower[18 + i];
			csflag[10000 + i] = cs.passable_data_upper[i];
		}
	} else {
		// Not doing chipset search, set defaults compatible with older lmu2png versions
		memset(csflag + 10000, 0x10, 144);
	}

	SDL_Surface* output_img = SDL_CreateRGBSurface(0, map->width * 16, map->height * 16, 32, 0, 0, 0, 0);

	if (!output_img) {
		std::cout << "Unable to create output image." << std::endl;
		exit(EXIT_FAILURE);
	}

	if (show_events) {
		// Just do the Y-sort here. Yes, it modifies the data that's supposed to be rendered.
		// Doesn't particularly matter. What does matter is that this has to be a stable_sort,
		//  so equivalent Y still causes ID order to be prioritized (just in case)
		std::stable_sort(map->events.begin(), map->events.end(), MapEventYSort);
	}

	RenderCore(output_img, chipset, csflag, map, show_background, show_lowertiles, show_uppertiles, show_events);

	if (IMG_SavePNG(output_img, output.c_str()) < 0) {
		std::cout << IMG_GetError() << std::endl;
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
