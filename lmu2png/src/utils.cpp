/* utils.cpp
   Copyright (C) 2016-2023 EasyRPG Project <https://github.com/EasyRPG/>.

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
#include <SDL.h>
#include <SDL_image.h>
#include <lcf/ldb/reader.h>
#include <lcf/lmu/reader.h>
#include <lcf/reader_lcf.h>
#include <lcf/rpg/map.h>
#include <lcf/rpg/chipset.h>
#include "utils.h"
#include "chipset.h"
#include "sdlxyz.h"

static std::vector<std::string> resource_dirs = {};

std::string GetFileDirectory(const std::string& file) {
	size_t found = file.find_last_of("/\\");
	return found == std::string::npos ? "./" : file.substr(0,found + 1);
}

bool Exists(const std::string& filename) {
	std::ifstream infile(filename.c_str());
	return infile.good();
}

void CollectResourcePaths(std::string& main_path) {
	auto split_path = [](std::string path) {
		int start = 0;
		for (int i = 0; i <= path.size(); i++) {
			if (path[i] == ';' || path[i] == ':' || i == path.size()) {
				std::string temp;
				temp.append(path, start, i - start);
				resource_dirs.emplace_back(temp);
				start = i + 1;
			}
		}
	};

	resource_dirs.clear();
	resource_dirs.emplace_back(main_path);

	char* rtp2k_ptr = getenv("RPG2K_RTP_PATH");
	if (rtp2k_ptr)
		split_path(rtp2k_ptr);

	char* rtp2k3_ptr = getenv("RPG2K3_RTP_PATH");
	if (rtp2k3_ptr)
		split_path(rtp2k3_ptr);
}

std::string FindResource(const std::string& folder, const std::string& base_name) {
	for (const auto& dir : resource_dirs) {
		for (const auto& ext : {".png", ".bmp", ".xyz"}) {
			if (Exists(dir + "/" + folder + "/" + base_name + ext))
				return dir + "/" + folder + "/" + base_name + ext;
		}
	}
	return "";
}

SDL_Surface* LoadImage(const char* image_path, bool transparent) {
	// Try XYZ, then IMG_Load
	SDL_Surface* image = LoadImageXYZ(image_path);
	if (!image) {
		image = IMG_Load(image_path);
	}
	if (!image) {
		std::cerr << IMG_GetError() << "\n";
		return nullptr;
	}

	if (transparent && image->format->palette) {
		// Set as color key the first color in the palette
		SDL_SetColorKey(image, SDL_TRUE, 0);
	}

	return image;
}

void DrawTiles(SDL_Surface* output_img, stChipset * gen, uint8_t * csflag, std::unique_ptr<lcf::rpg::Map> & map, ImgConfig conf, int flaglayer) {
	for (int y = 0; y < map->height; ++y) {
		for (int x = 0; x < map->width; ++x) {
			// Different logic between these.
			int tindex = x + y * map->width;
			if (!conf.no_lowertiles) {
				uint16_t tid = map->lower_layer[tindex];
				int l = (csflag[tid] & 0x30) ? 1 : 0;
				if (l == flaglayer)
					gen->RenderTile(output_img, x*16, y*16, map->lower_layer[x+y*map->width], 0);
			}
			if (!conf.no_uppertiles) {
				uint16_t tid = map->upper_layer[tindex];
				int l = (csflag[tid] & 0x10) ? 1 : 0;
				if (l == flaglayer)
					gen->RenderTile(output_img, x*16, y*16, map->upper_layer[x+y*map->width], 0);
			}
		}
	}
}

void DrawEvents(SDL_Surface* output_img, stChipset * gen, std::unique_ptr<lcf::rpg::Map> & map, int layer, ImgConfig conf) {
	for (const lcf::rpg::Event& ev : map->events) {
		const lcf::rpg::EventPage* evp = nullptr;
		// Find highest page without conditions
		if (conf.ignore_conditions)
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
				std::cerr << "Can't find charset " << evp->character_name << "\n";
				continue;
			}

			SDL_Surface* charset_img(LoadImage(charset.c_str(), true));
			int frame = evp->character_pattern;
			if (conf.simulate_movement &&
				(evp->animation_type == 0 || evp->animation_type == 2 || evp->animation_type == 6)) {
				// middle frame
				frame = 1;
			}
			SDL_Rect src_rect = {(evp->character_index % 4) * 72 + frame * 24,
				(evp->character_index / 4) * 128 + evp->character_direction * 32, 24, 32};
			SDL_Rect dst_rect {ev.x * 16 - 4, ev.y * 16 - 16, 16, 32};
			SDL_BlitSurface(charset_img, &src_rect, output_img, &dst_rect);
		}
	}
}

void RenderCore(SDL_Surface* output_img, std::string chipset, uint8_t * csflag, std::unique_ptr<lcf::rpg::Map> & map, ImgConfig conf) {
	SDL_Surface* chipset_img;
	if (!chipset.empty()) {
		chipset_img = LoadImage(chipset.c_str(), true);
	} else {
		chipset_img = SDL_CreateRGBSurfaceWithFormat(0, 32 * 16, 45 * 16, 32, SDL_PIXELFORMAT_RGBA32);
	}

	stChipset gen;
	gen.GenerateFromSurface(chipset_img);

	// Draw parallax background
	if (!conf.no_background) {
		std::string pname = lcf::ToString(map->parallax_name);
		if(!pname.empty()) {
			std::string background(FindResource("Panorama", pname));
			if (background.empty()) {
				std::cerr << "Can't find parallax background " << map->parallax_name << "\n";
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
	}

	// Draw below tile layer
	if (!(conf.no_lowertiles && conf.no_uppertiles))
		DrawTiles(output_img, &gen, csflag, map, conf, 0);
	// Draw below-player & player-level events
	if (!conf.no_events) {
		DrawEvents(output_img, &gen, map, 0, conf);
		DrawEvents(output_img, &gen, map, 1, conf);
	}
	// Draw above tile layer
	if (!(conf.no_lowertiles && conf.no_uppertiles))
		DrawTiles(output_img, &gen, csflag, map, conf, 1);
	// Draw events
	if (!conf.no_events)
		DrawEvents(output_img, &gen, map, 2, conf);
}
