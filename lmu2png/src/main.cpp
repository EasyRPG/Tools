/* main.cpp, lmu2png main file.
   Copyright (C) 2016-2024 EasyRPG Project <https://github.com/EasyRPG/>.

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
#include <map>
#include <algorithm>
#include <argparse.hpp>
#include <FreeImage.h>
#include <lcf/ldb/reader.h>
#include <lcf/lmu/reader.h>
#include <lcf/reader_lcf.h>
#include <lcf/rpg/map.h>
#include <lcf/rpg/chipset.h>
#include "chipset.h"
#include "xyzplugin.h"

// type and other definitions

enum class LAYER : int {
	LOWER = 0,
	UPPER,
	EVENTS
};

using sOpts = struct {
	bool verbose;
	bool no_background;
	bool no_lowertiles;
	bool no_uppertiles;
	bool no_events;
	bool ignore_conditions;
	bool simulate_movement;
};

using CharsetCacheMap = std::map<std::string, BitmapPtr>;

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

void DrawTiles(FIBITMAP* output_img, Chipset* gen, uint8_t * csflag, std::unique_ptr<lcf::rpg::Map> & map, sOpts opts, LAYER flaglayer) {
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

void DrawEvents(FIBITMAP* output_img, Chipset* gen, std::unique_ptr<lcf::rpg::Map> & map, LAYER layer, CharsetCacheMap &charsets, sOpts opts) {
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
			if(charsets.count(cname) == 0) {
				if(opts.verbose)
					std::cerr << "Loading CharSet \"" << cname << "\"\n";

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
				if(opts.verbose)
					std::cerr << "Using CharSet \"" << cname << "\" (cached)\n";
#endif
			}

			int frame = evp->character_pattern;
			if (opts.simulate_movement &&
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

void RenderCore(FIBITMAP* output_img, std::string &chipset, uint8_t * csflag, std::unique_ptr<lcf::rpg::Map> & map, sOpts opts) {
	BitmapPtr chipset_img;
	if (!chipset.empty()) {
		chipset_img.reset(LoadImage(chipset, true));
	}

	if(!chipset_img) {
		if(opts.verbose)
			std::cerr << "Using empty chipset image.\n";

		chipset_img.reset(FreeImage_Allocate(CHIPSET_WIDTH, CHIPSET_HEIGHT, 32));
		if (!chipset_img) {
			std::cout << "Unable to create empty chipset image.\n";
			exit(EXIT_FAILURE);
		}
	}
	Chipset gen(chipset_img.get());

	// Draw parallax background
	if (!opts.no_background) {
		std::string pname = lcf::ToString(map->parallax_name);
		if (pname.empty()) {
			if(opts.verbose)
				std::cerr << "Using black background.\n";

			// Fill screen with black
			RGBQUAD black{0, 0, 0, 0xFF};
			FreeImage_FillBackground(output_img, &black);
		} else {
			if(opts.verbose)
				std::cerr << "Loading Panorama \"" << pname << "\"\n";

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
	if (!(opts.no_lowertiles && opts.no_uppertiles))
		DrawTiles(output_img, &gen, csflag, map, opts, LAYER::LOWER);
	// Draw below-player & player-level events
	if (!opts.no_events) {
		DrawEvents(output_img, &gen, map, LAYER::LOWER, charsets, opts);
		DrawEvents(output_img, &gen, map, LAYER::UPPER, charsets, opts);
	}
	// Draw above tile layer
	if (!(opts.no_lowertiles && opts.no_uppertiles))
		DrawTiles(output_img, &gen, csflag, map, opts, LAYER::UPPER);
	// Draw events
	if (!opts.no_events)
		DrawEvents(output_img, &gen, map, LAYER::EVENTS, charsets, opts);

	//for(auto &kv : charsets) {
	//	FreeImage_Save(FIF_PNG, kv.second.get(), std::string("_cs_" + kv.first + ".png").c_str());
	//}
}

void MyFreeImageMessageHandler(FREE_IMAGE_FORMAT /* fif */, const char *message) {
	std::cout << "FreeImage error: " << message << "\n";
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
	cli.add_argument("--verbose").store_into(opts.verbose)
		.help("Explain what is being done").flag();

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

	// Static FreeImage library needs this
	FreeImage_Initialise(false);
	atexit(FreeImage_DeInitialise);
	// Register our error handler and plugin
	FreeImage_SetOutputMessage(MyFreeImageMessageHandler);
	FreeImage_RegisterLocalPlugin(InitXYZ);

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

		if(opts.verbose)
			std::cerr << "Loading ChipSet \"" << chipset_base << "\"\n";

		chipset = FindResource("ChipSet", chipset_base);
		if (chipset.empty() && !chipset_base.empty()) {
			std::cout << "Chipset \"" << chipset_base << "\" not found.\n";
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

	BitmapPtr output_img{FreeImage_Allocate(map->width * TILE_SIZE, map->height * TILE_SIZE, 32)};
	if (!output_img) {
		std::cout << "Unable to create output image.\n";
		exit(EXIT_FAILURE);
	}

	if (!opts.no_events) {
		// Just do the Y-sort here. Yes, it modifies the data that's supposed to be rendered.
		// Doesn't particularly matter. What does matter is that this has to be a stable_sort,
		// so equivalent Y still causes ID order to be prioritized (just in case)
		std::stable_sort(map->events.begin(), map->events.end(),
			[](const auto& ev1, const auto& ev2) { return ev1.y < ev2.y; });
	}

	RenderCore(output_img.get(), chipset, csflag, map, opts);

	if (!FreeImage_Save(FIF_PNG, output_img.get(), output.c_str(), PNG_Z_BEST_COMPRESSION)) {
		std::cout << "Error saving \"" << output << "\".\n";
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
