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
#include <lcf/ldb/reader.h>
#include <lcf/lmu/reader.h>
#include <lcf/reader_lcf.h>
#include <lcf/rpg/map.h>
#include <lcf/rpg/chipset.h>
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

void DrawTiles(SDL_Surface* output_img, stChipset * gen, uint8_t * csflag, std::unique_ptr<lcf::rpg::Map> & map, int show_lowertiles, int show_uppertiles, int flaglayer) {
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

void DrawEvents(SDL_Surface* output_img, stChipset * gen, std::unique_ptr<lcf::rpg::Map> & map, int layer, bool ignore_conditions, bool simulate_movement) {
	for (const lcf::rpg::Event& ev : map->events) {
		const lcf::rpg::EventPage* evp = nullptr;
		// Find highest page without conditions
		if (ignore_conditions)
			evp = &ev.pages[0];
		else {
			for (int i = 0; i < (int)ev.pages.size(); ++i) {
				const auto& flg = ev.pages[i].condition.flags;
				if (flg.switch_a || flg.switch_b || flg.variable || flg.item || flg.actor || flg.timer || flg.timer2)
					continue;
				evp = &ev.pages[i];
			}
		}
		if (!evp)
				continue;
		// Event layering
		if (evp->layer >= 0 && evp->layer < 3 && evp->layer != layer)
			continue;
		if (evp->character_name.empty())
			gen->RenderTile(output_img, (ev.x)*16, (ev.y)*16, 0x2710 + evp->character_index, 0);
		else {
			std::string cname = lcf::ToString(evp->character_name);
			std::string charset(FindResource("CharSet", cname));
			if (charset.empty()) {
				std::cout << "Can't find charset " << evp->character_name << std::endl;
				continue;
			}

			SDL_Surface* charset_img(LoadImage(charset.c_str(), true));
			SDL_Rect src_rect;
			if (!simulate_movement || (simulate_movement && evp->animation_type))
				src_rect = {(evp->character_index % 4) * 72 + evp->character_pattern * 24,
					 (evp->character_index / 4) * 128 + evp->character_direction * 32, 24, 32};
			else
				src_rect = {(evp->character_index % 4) * 72 + 24,
					 (evp->character_index / 4) * 128 + evp->character_direction * 32, 24, 32};
			SDL_Rect dst_rect {ev.x * 16 - 4, ev.y * 16 - 16, 16, 32}; // Why -4 and -16?
			SDL_BlitSurface(charset_img, &src_rect, output_img, &dst_rect);
		}
	}
}

void RenderCore(SDL_Surface* output_img, std::string chipset, uint8_t * csflag, std::unique_ptr<lcf::rpg::Map> & map, int show_background, int show_lowertiles, int show_uppertiles, int show_events, bool ignore_conditions, bool simulate_movement) {
	SDL_Surface* chipset_img;
	if (!chipset.empty()) {
		chipset_img = LoadImage(chipset.c_str(), true);
	} else {
		chipset_img = SDL_CreateRGBSurfaceWithFormat(0, 32 * 16, 45 * 16, 32, SDL_PIXELFORMAT_RGBA32);
	}

	stChipset gen;
	gen.GenerateFromSurface(chipset_img);

	// Draw parallax background
	if (show_background) {
		
		std::string pname = lcf::ToString(map->parallax_name);
		std::string background(FindResource("Panorama", pname));
		if (background.empty() && !map->parallax_name.empty()) {
			std::cout << "Can't find parallax background " << map->parallax_name << std::endl;
		} else {
			SDL_Surface* background_img;
			if (map->parallax_name.empty()) {
				background_img = SDL_CreateRGBSurface(0, 1, 1, 24, 0, 0, 0, 0);
				SDL_FillRect(background_img, 0, 0);
			} else {
				background_img = (LoadImage(background.c_str()));
			}
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
		DrawEvents(output_img, &gen, map, 0, ignore_conditions, simulate_movement);
		DrawEvents(output_img, &gen, map, 1, ignore_conditions, simulate_movement);
	}
	// Draw above tile layer
	if (show_lowertiles || show_uppertiles)
		DrawTiles(output_img, &gen, csflag, map, show_lowertiles, show_uppertiles, 1);
	// Draw events
	if (show_events)
		DrawEvents(output_img, &gen, map, 2, ignore_conditions, simulate_movement);
}

bool MapEventYSort(const lcf::rpg::Event& ev1, const lcf::rpg::Event& ev2) {
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
	bool ignore_conditions = false;
	bool simulate_movement = false;
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
		} else if (arg == "--no-background" || arg == "-nb") {
			show_background = false;
		} else if (arg == "--no-lowertiles" || arg == "-nl") {
			show_lowertiles = false;
		} else if (arg == "--no-uppertiles" || arg == "-nu") {
			show_uppertiles = false;
		} else if (arg == "--no-events" || arg == "-ne") {
			show_events = false;
		} else if (arg == "--ignore-conditions" || arg == "-ic") {
			ignore_conditions = true;
		} else if (arg == "--simulate-movement" || arg == "-sm") {
			simulate_movement = true;
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
		encoding = lcf::ReaderUtil::GetEncoding(path + "RPG_RT.ini");

	std::unique_ptr<lcf::rpg::Map> map(lcf::LMU_Reader::Load(input, encoding));
	if (!map) {
		std::cout << lcf::LcfReader::GetError() << std::endl;
		exit(EXIT_FAILURE);
	}

	if (output.empty()) {
		output = input.substr(0, input.length() - 3) + "png";
	}

	if (chipset.empty()) {
		// Get chipset from database
		if (database.empty())
			database = path + "RPG_RT.ldb";

		auto db = lcf::LDB_Reader::Load(database, encoding);

		if (!db) {
			std::cout << lcf::LcfReader::GetError() << std::endl;
			exit(EXIT_FAILURE);
		}

		assert(map->chipset_id <= (int)db->chipsets.size());
		lcf::rpg::Chipset & cs = db->chipsets[map->chipset_id - 1];
		std::string chipset_base(cs.chipset_name);

		chipset = FindResource("ChipSet", chipset_base);
		if (chipset.empty() && !chipset_base.empty()) {
			std::cout << "Chipset " << chipset_base << " can't be found." << std::endl;
			exit(EXIT_FAILURE);
		}
		// Load flags.
		// The first 18 in lower cover various zones.
		// Water A/B/C
		for (int i = 0; i < 3; i++)
			memset(csflag + (1000 * i), cs.passable_data_lower[i], 1000);
		// Animated tiles, made up of 3 sets of 50.
		for (int i = 0; i < 3; i++)
			memset(csflag + 3000 + (i * 50), cs.passable_data_lower[3 + i], 50);
		// Terrain ATs, made up of 12 sets of 50.
		for (int i = 0; i < 12; i++)
			memset(csflag + 4000 + (i * 50), cs.passable_data_lower[6 + i], 50);
		// Lower/upper 144-tile pages, made up of 144 individual flag bytes per page.
		for (int i = 0; i < 144; i++) {
			csflag[5000 + i] = cs.passable_data_lower[18 + i];
			csflag[10000 + i] = cs.passable_data_upper[i];
		}
	} else {
		// Not doing chipset search, set defaults compatible with older lmu2png versions
		memset(csflag + 10000, 0x10, 144);
	}

	SDL_Surface* output_img = SDL_CreateRGBSurfaceWithFormat(0, map->width * 16, map->height * 16, 32, SDL_PIXELFORMAT_RGBA32);

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

	RenderCore(output_img, chipset, csflag, map, show_background, show_lowertiles, show_uppertiles, show_events, ignore_conditions, simulate_movement);

	if (IMG_SavePNG(output_img, output.c_str()) < 0) {
		std::cout << IMG_GetError() << std::endl;
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
