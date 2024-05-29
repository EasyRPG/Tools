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
#include <argparse.hpp>
#include <SDL_image.h>
#include "sdlxyz.h"
#include <lcf/ldb/reader.h>
#include <lcf/lmu/reader.h>
#include <lcf/reader_lcf.h>
#include <lcf/rpg/map.h>
#include <lcf/rpg/chipset.h>
#include "chipset.h"

// prevent SDL main rename
#undef main

enum class LAYER : int {
	LOWER = 0,
	UPPER,
	EVENTS
};

using sOpts = struct {
	bool no_background;
	bool no_lowertiles;
	bool no_uppertiles;
	bool no_events;
	bool ignore_conditions;
	bool simulate_movement;
};

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

SDL_Surface* LoadImage(std::string &image_path, bool transparent = false) {
	// Try XYZ, then IMG_Load
	SDL_Surface* image = LoadImageXYZ(image_path.c_str());
	if (!image) {
		image = IMG_Load(image_path.c_str());
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

void DrawTiles(SDL_Surface* output_img, Chipset * gen, uint8_t * csflag, std::unique_ptr<lcf::rpg::Map> & map, sOpts opts, LAYER flaglayer) {
	for (int y = 0; y < map->height; ++y) {
		for (int x = 0; x < map->width; ++x) {
			// Different logic between these.
			int tindex = x + y * map->width;
			if (!opts.no_lowertiles) {
				uint16_t tid = map->lower_layer[tindex];
				LAYER l = (csflag[tid] & 0x30) ? LAYER::UPPER : LAYER::LOWER;
				if (l == flaglayer)
					gen->RenderTile(output_img, x, y, map->lower_layer[x+y*map->width], 0);
			}
			if (!opts.no_uppertiles) {
				uint16_t tid = map->upper_layer[tindex];
				LAYER l = (csflag[tid] & 0x10) ? LAYER::UPPER : LAYER::LOWER;
				if (l == flaglayer)
					gen->RenderTile(output_img, x, y, map->upper_layer[x+y*map->width], 0);
			}
		}
	}
}

void DrawEvents(SDL_Surface* output_img, Chipset * gen, std::unique_ptr<lcf::rpg::Map> & map, LAYER layer, sOpts opts) {
	for (const lcf::rpg::Event& ev : map->events) {
		const lcf::rpg::EventPage* evp = nullptr;

		if (opts.ignore_conditions)
			evp = &ev.pages[0];
		else {
			// Find highest page without conditions
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
		if (evp->layer >= static_cast<int>(LAYER::LOWER) && evp->layer <= static_cast<int>(LAYER::EVENTS)
			&& evp->layer != static_cast<int>(layer))
			continue;

		if (evp->character_name.empty())
			gen->RenderTile(output_img, ev.x, ev.y, TILETYPE::UPPER + evp->character_index, 0);
		else {
			std::string cname = lcf::ToString(evp->character_name);
			std::string charset(FindResource("CharSet", cname));
			if (charset.empty()) {
				std::cout << "Can't find charset " << evp->character_name << std::endl;
				continue;
			}

			SDL_Surface* charset_img{LoadImage(charset, true)};
			int frame = evp->character_pattern;
			if (opts.simulate_movement &&
				(evp->animation_type == 0 || evp->animation_type == 2 || evp->animation_type == 6)) {
				// middle frame
				frame = 1;
			}
			SDL_Rect src_rect = {(evp->character_index % 4) * 72 + frame * 24,
				(evp->character_index / 4) * 128 + evp->character_direction * 32, 24, 32};
			SDL_Rect dst_rect {ev.x * TILE_SIZE - 4, ev.y * TILE_SIZE - 16, 16, 32}; // Why -4 and -16?
			SDL_BlitSurface(charset_img, &src_rect, output_img, &dst_rect);
		}
	}
}

void RenderCore(SDL_Surface* output_img, std::string chipset, uint8_t * csflag, std::unique_ptr<lcf::rpg::Map> & map, sOpts opts) {
	SDL_Surface* chipset_img;
	if (!chipset.empty()) {
		chipset_img = LoadImage(chipset, true);
	} else {
		chipset_img = SDL_CreateRGBSurfaceWithFormat(0, CHIPSET_WIDTH, CHIPSET_HEIGHT, 32, SDL_PIXELFORMAT_RGBA32);
	}
	Chipset gen(chipset_img);

	// Draw parallax background
	if (!opts.no_background) {
		std::string pname = lcf::ToString(map->parallax_name);
		std::string background(FindResource("Panorama", pname));
		if (background.empty() && !map->parallax_name.empty()) {
			std::cout << "Can't find parallax background " << map->parallax_name << std::endl;
		} else {
			SDL_Surface* background_img;
			if (map->parallax_name.empty()) {
				background_img = SDL_CreateRGBSurface(0, 1, 1, 24, 0, 0, 0, 0);
				SDL_FillRect(background_img, nullptr, 0);
			} else {
				background_img = LoadImage(background);
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
	if (!(opts.no_lowertiles && opts.no_uppertiles))
		DrawTiles(output_img, &gen, csflag, map, opts, LAYER::LOWER);
	// Draw below-player & player-level events
	if (!opts.no_events) {
		DrawEvents(output_img, &gen, map, LAYER::LOWER, opts);
		DrawEvents(output_img, &gen, map, LAYER::UPPER, opts);
	}
	// Draw above tile layer
	if (!(opts.no_lowertiles && opts.no_uppertiles))
		DrawTiles(output_img, &gen, csflag, map, opts, LAYER::UPPER);
	// Draw events
	if (!opts.no_events)
		DrawEvents(output_img, &gen, map, LAYER::EVENTS, opts);
}

int main(int argc, char** argv) {
	sOpts opts = { 0 };
	std::string database, chipset, encoding, output, input;

	// add usage and help messages
	argparse::ArgumentParser cli("lmu2png", PACKAGE_VERSION);
	cli.set_usage_max_line_width(120);
	cli.add_description("EasyRPG lmu2png - Render a map to PNG file");
	cli.add_epilog("Homepage " PACKAGE_URL " - Report bugs at: " PACKAGE_BUGREPORT);

	// Parse arguments
	cli.add_argument("mapfile").required().store_into(input)
		.help("Map file to render")
		.metavar("MapXXXX.lmu");
	cli.add_argument("-e", "--encoding").store_into(encoding)
		.help("Project encoding (defaults to autodetection)")
		.metavar("ENC");
	cli.add_argument("-d", "--database").store_into(database)
		.help("Database file to use; if unspecified, uses the database file\n"
			"in the map folder").metavar("LDB");
	cli.add_argument("-c", "--chipset").store_into(chipset)
		.help("Chipset file to use; if unspecified, will be read from\n"
			"the database").metavar("IMG");
	cli.add_argument("-o", "--output").store_into(output)
		.help("Set the output filepath (defaults to map name)")
		.metavar("PNG");

	cli.add_group("Graphic Options");
	cli.add_argument("-B", "--no-background").store_into(opts.no_background)
		.help("Do not draw the parallax background").flag();
	cli.add_argument("-L", "--no-lowertiles").store_into(opts.no_lowertiles)
		.help("Do not draw lower layer tiles").flag();
	cli.add_argument("-U", "--no-uppertiles").store_into(opts.no_uppertiles)
		.help("Do not draw upper layer tiles").flag();
	cli.add_argument("-E", "--no-events").store_into(opts.no_events)
		.help("Do not draw events").flag();
	cli.add_argument("-C", "--ignore-conditions").store_into(opts.ignore_conditions)
		.help("Always draw the first page of the event instead of finding\n"
			"the first page with no conditions").flag();
	cli.add_argument("-M", "--simulate-movement").store_into(opts.simulate_movement)
		.help("For event pages with certain animation types, draw the middle\n"
			"frame instead of the frame specified for the page").flag();

	try {
		cli.parse_args(argc, argv);
	} catch (const std::exception& err) {
		std::cerr << err.what() << std::endl;
		// print usage message
		std::cerr << cli.usage() << std::endl;
		std::exit(EXIT_FAILURE);
	}

	if (!Exists(input)) {
		std::cout << "Input map file " << input << " not found." << std::endl;
		exit(EXIT_FAILURE);
	}

	// ChipSet flags
	uint8_t csflag[65536];
	memset(csflag, 0, 65536);

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
			std::cout << "Chipset " << chipset_base << " not found." << std::endl;
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

	SDL_Surface* output_img = SDL_CreateRGBSurfaceWithFormat(0, map->width * TILE_SIZE, map->height * TILE_SIZE, 32, SDL_PIXELFORMAT_RGBA32);
	if (!output_img) {
		std::cout << "Unable to create output image." << std::endl;
		exit(EXIT_FAILURE);
	}

	if (!opts.no_events) {
		// Just do the Y-sort here. Yes, it modifies the data that's supposed to be rendered.
		// Doesn't particularly matter. What does matter is that this has to be a stable_sort,
		// so equivalent Y still causes ID order to be prioritized (just in case)
		std::stable_sort(map->events.begin(), map->events.end(),
			[](const auto& ev1, const auto& ev2) { return ev1.y < ev2.y; });
	}

	RenderCore(output_img, chipset, csflag, map, opts);

	if (IMG_SavePNG(output_img, output.c_str()) < 0) {
		std::cout << IMG_GetError() << std::endl;
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
