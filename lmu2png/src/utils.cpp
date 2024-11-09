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
#include <map>
#include <lcf/ldb/reader.h>
#include <lcf/lmu/reader.h>
#include <lcf/reader_lcf.h>
#include <lcf/rpg/map.h>
#include <lcf/rpg/chipset.h>
#include "utils.h"
#include "chipset.h"

using CharsetCacheMap = std::map<std::string, BitmapPtr>;

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
	if (rtp2k_ptr) {
		split_path(rtp2k_ptr);
	}

	char* rtp2k3_ptr = getenv("RPG2K3_RTP_PATH");
	if (rtp2k3_ptr) {
		split_path(rtp2k3_ptr);
	}
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

void CustomAlphaCombine(FIBITMAP *src, int sLeft, int sTop, FIBITMAP *dst, int dLeft, int dTop, int width, int height) {
	if (!src || !dst) {
		std::cout << "Source or Destination parameter undefined.\n";
		return;
	}

	int sBpp = FreeImage_GetBPP(src);
	int dBpp = FreeImage_GetBPP(dst);
	if((sBpp != 32) || (dBpp != 32)) {
		std::cout << "Source or Destination have wrong format.\n";
		return;
	}

	int sWidth = FreeImage_GetWidth(src);
	int sHeight = FreeImage_GetHeight(src);
	int dWidth = FreeImage_GetWidth(dst);
	int dHeight = FreeImage_GetHeight(dst);

	// Sanitize src dims
	if((sLeft < 0) || (sTop < 0) || (sLeft + width > sWidth) || (sTop + height > sHeight)) {
		std::cout << "Source dimension error.\n";
		return;
	}

	// Event special case: draw only part on borders
	if(dLeft < 0) {
		int amount = dLeft + width;
		dLeft = 0;
		width = amount;
	}
	if(dTop < 0) {
		int amount = dTop + height;
		dTop = 0;
		height = amount;
	}
	if(dLeft + width > dWidth) {
		int amount = dLeft + width - dWidth;
		width -= amount;
	}
	if(dTop + height > dHeight) {
		int amount = dTop + height - dHeight;
		height -= amount;
	}

	// Sanitize final dims
	if ((width <= 0) || (height <= 0)) {
		std::cout << "Skipping zero/negative size copy.\n";
		return;
	}

	int bytespp = sBpp / 8;
	for(int y = 0; y < height; y++) {
		// set position to upper left corner
		BYTE *src_bits = FreeImage_GetScanLine(src, sHeight - 1 - (sTop + y)) + sLeft * bytespp;
		BYTE *dst_bits = FreeImage_GetScanLine(dst, dHeight - 1 - (dTop + y)) + dLeft * bytespp;

		for(int x = 0; x < width; x++) {
			// skip fully transparent pixels
			if (src_bits[FI_RGBA_ALPHA] != 0) {
				dst_bits[FI_RGBA_RED]   = src_bits[FI_RGBA_RED];
				dst_bits[FI_RGBA_GREEN] = src_bits[FI_RGBA_GREEN];
				dst_bits[FI_RGBA_BLUE]  = src_bits[FI_RGBA_BLUE];
				dst_bits[FI_RGBA_ALPHA] = src_bits[FI_RGBA_ALPHA];
			}

			// next pixel
			src_bits += bytespp;
			dst_bits += bytespp;
		}
	}
}

FIBITMAP* LoadImage(std::string &image_path, bool transparent = false) {
	BitmapPtr image;

	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(image_path.c_str());
	if (format != FIF_UNKNOWN) {
		image.reset(FreeImage_Load(format, image_path.c_str()));
	}

	if (!image) {
		std::cout << "Error loading \"" << image_path << "\".\n";
		return nullptr;
	}

	if (transparent) {
		// Set as color key the first color in the palette
		FreeImage_SetTransparentIndex(image.get(), 0);
	}

	// To make blitting easier, unpack palette and add alpha channel
	FIBITMAP *output = FreeImage_ConvertTo32Bits(image.get());

	return output;
}

void DrawTiles(FIBITMAP* output_img, Chipset* gen, uint8_t * csflag, std::unique_ptr<lcf::rpg::Map> & map, L2IConfig conf, LAYER flaglayer) {
	for (int y = 0; y < map->height; ++y) {
		for (int x = 0; x < map->width; ++x) {
			// Different logic between these.
			int tindex = x + y * map->width;

			if (!conf.no_lowertiles) {
				uint16_t tid = map->lower_layer[tindex];
				LAYER l = (csflag[tid] & 0x30) ? LAYER::UPPER : LAYER::LOWER;
				if (l == flaglayer)
					gen->RenderTile(output_img, x, y, map->lower_layer[x+y*map->width], 0);
			}

			if (!conf.no_uppertiles) {
				uint16_t tid = map->upper_layer[tindex];
				LAYER l = (csflag[tid] & 0x10) ? LAYER::UPPER : LAYER::LOWER;
				if (l == flaglayer)
					gen->RenderTile(output_img, x, y, map->upper_layer[x+y*map->width], 0);
			}
		}
	}
}

void DrawEvents(FIBITMAP* output_img, Chipset* gen, std::unique_ptr<lcf::rpg::Map> & map, LAYER layer, CharsetCacheMap &charsets, L2IConfig conf) {
	for (const lcf::rpg::Event& ev : map->events) {
		const lcf::rpg::EventPage* evp = nullptr;

		if (conf.ignore_conditions) {
			evp = &ev.pages[0];
		} else {
			// Find highest page without conditions
			for (int i = 0; i < (int)ev.pages.size(); ++i) {
				const auto& flg = ev.pages[i].condition.flags;
				if (flg.switch_a || flg.switch_b || flg.variable || flg.item || flg.actor || flg.timer || flg.timer2)
					continue;
				evp = &ev.pages[i];
			}
		}
		if (!evp) {
			continue;
		}

		// Event layering
		if (evp->layer >= static_cast<int>(LAYER::LOWER)
			&& evp->layer <= static_cast<int>(LAYER::EVENTS)
			&& evp->layer != static_cast<int>(layer))
			continue;

		if (evp->character_name.empty()) {
			gen->RenderTile(output_img, ev.x, ev.y, TILETYPE::UPPER + evp->character_index, 0);
		} else {
			std::string cname = lcf::ToString(evp->character_name);
			if(charsets.count(cname) == 0) {
				if(conf.verbose) {
					std::cerr << "Loading CharSet \"" << cname << "\"\n";
				}

				std::string charset{FindResource("CharSet", cname)};
				if (charset.empty()) {
					std::cout << "Charset \"" << evp->character_name << "\" not found.\n";
					continue;
				}

				// add image to cache
				BitmapPtr charset_img{LoadImage(charset, true)};
				if(charset_img) {
					charsets.emplace(cname, std::move(charset_img));
				}
			} else {
				// use from cache
#ifndef NDEBUG
				if(conf.verbose) {
					std::cerr << "Using CharSet \"" << cname << "\" (cached)\n";
				}
#endif
			}

			int frame = evp->character_pattern;
			if (conf.simulate_movement &&
				(evp->animation_type == 0 || evp->animation_type == 2 || evp->animation_type == 6)) {
				// middle frame
				frame = 1;
			}

			CustomAlphaCombine(charsets[cname].get(), (evp->character_index % 4) * 72 + frame * 24,
				(evp->character_index / 4) * 128 + evp->character_direction * 32,
				output_img, ev.x * TILE_SIZE-4, ev.y * TILE_SIZE-16, // Why -4 and -16?
				24, 32);
		}
	}
}

void RenderCore(FIBITMAP* output_img, uint8_t * csflag, std::unique_ptr<lcf::rpg::Map> & map, L2IConfig conf) {
	BitmapPtr chipset_img;
	if (!conf.chipset.empty()) {
		chipset_img.reset(LoadImage(conf.chipset, true));
	}

	if(!chipset_img) {
		if(conf.verbose) {
			std::cerr << "Using empty chipset image.\n";
		}

		chipset_img.reset(FreeImage_Allocate(CHIPSET_WIDTH, CHIPSET_HEIGHT, 32));
		if (!chipset_img) {
			std::cout << "Unable to create empty chipset image.\n";
			exit(EXIT_FAILURE);
		}
	}
	Chipset gen(chipset_img.get());

	// Draw parallax background
	if (!conf.no_background) {
		std::string pname = lcf::ToString(map->parallax_name);
		if (pname.empty()) {
			if(conf.verbose) {
				std::cerr << "Using black background.\n";
			}

			// Fill screen with black
			RGBQUAD black{0, 0, 0, 0xFF};
			FreeImage_FillBackground(output_img, &black);
		} else {
			if(conf.verbose) {
				std::cerr << "Loading Panorama \"" << pname << "\"\n";
			}

			std::string background{FindResource("Panorama", pname)};
			if (background.empty()) {
				std::cout << "Parallax background \"" << pname << "\" not found.\n";
			} else {
				BitmapPtr background_img{LoadImage(background)};

				// Fill screen with scaled background
				int dw = FreeImage_GetWidth(output_img);
				int dh = FreeImage_GetHeight(output_img);
				BitmapPtr scaled{FreeImage_Rescale(background_img.get(), dw, dh, FILTER_BICUBIC)};
				FreeImage_Paste(output_img, scaled.get(), 0, 0, 256);

				//FreeImage_Save(FIF_PNG, scaled.get(), "_back.png");
			}
		}
	}

	CharsetCacheMap charsets;

	// Draw below tile layer
	if (!(conf.no_lowertiles && conf.no_uppertiles)) {
		DrawTiles(output_img, &gen, csflag, map, conf, LAYER::LOWER);
	}
	// Draw below-player & player-level events
	if (!conf.no_events) {
		DrawEvents(output_img, &gen, map, LAYER::LOWER, charsets, conf);
		DrawEvents(output_img, &gen, map, LAYER::UPPER, charsets, conf);
	}
	// Draw above tile layer
	if (!(conf.no_lowertiles && conf.no_uppertiles)) {
		DrawTiles(output_img, &gen, csflag, map, conf, LAYER::UPPER);
	}
	// Draw events
	if (!conf.no_events) {
		DrawEvents(output_img, &gen, map, LAYER::EVENTS, charsets, conf);
	}

	//for(auto &kv : charsets) {
	//	FreeImage_Save(FIF_PNG, kv.second.get(), std::string("_cs_" + kv.first + ".png").c_str());
	//}
}
