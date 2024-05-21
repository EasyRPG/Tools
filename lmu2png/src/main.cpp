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
#include <argparse.hpp>
#include <SDL.h>
#include "main.h"
#include "utils.h"
#ifdef WITH_GUI
	#include "gui.h"
#endif

// prevent SDL main rename
#undef main

// internal functions
static SDL_Surface* process(ImgConfig conf, ErrorCallbackFunc error_cb, ErrorCallbackParam param = nullptr);
static void cliErrorCallback(const std::string& error, ErrorCallbackParam param = nullptr);

int main(int argc, char** argv) {
	std::string output;
	ImgConfig conf = {};

	// add usage and help messages
	argparse::ArgumentParser cli("lmu2png", PACKAGE_VERSION);
	cli.set_usage_max_line_width(120);
	cli.add_description("EasyRPG lmu2png - Render a map to PNG file");
	cli.add_epilog(
#ifdef WITH_GUI
		"Not providing a map file will open the graphical tool.\n\n"
#endif
		"Homepage " PACKAGE_URL " - Report bugs at: " PACKAGE_BUGREPORT);

	// Parse arguments
	cli.add_argument("mapfile").required().store_into(conf.map)
		.help("Map file to render")
		.metavar("MapXXXX.lmu");
	cli.add_argument("-e", "--encoding").store_into(conf.encoding)
		.help("Project encoding (defaults to autodetection)")
		.metavar("ENC");
	cli.add_argument("-d", "--database").store_into(conf.database)
		.help("Database file to use; if unspecified, uses the database file\n"
			"in the map folder").metavar("LDB");
	cli.add_argument("-c", "--chipset").store_into(conf.chipset)
		.help("Chipset file to use; if unspecified, will be read from\n"
			"the database").metavar("IMG");
	cli.add_argument("-o", "--output").store_into(output)
		.help("Set the output filepath (defaults to map name)")
		.metavar("PNG");

	cli.add_group("Graphic Options");
	cli.add_argument("-B", "--no-background").store_into(conf.no_background)
		.help("Do not draw the parallax background").flag();
	cli.add_argument("-L", "--no-lowertiles").store_into(conf.no_lowertiles)
		.help("Do not draw lower layer tiles").flag();
	cli.add_argument("-U", "--no-uppertiles").store_into(conf.no_uppertiles)
		.help("Do not draw upper layer tiles").flag();
	cli.add_argument("-E", "--no-events").store_into(conf.no_events)
		.help("Do not draw events").flag();
	cli.add_argument("-C", "--ignore-conditions").store_into(conf.ignore_conditions)
		.help("Always draw the first page of the event instead of finding\n"
			"the first page with no conditions").flag();
	cli.add_argument("-M", "--simulate-movement").store_into(conf.simulate_movement)
		.help("For event pages with certain animation types, draw the middle\n"
			"frame instead of the frame specified for the page").flag();

	try {
		cli.parse_args(argc, argv);
	} catch (const std::exception& err) {
#ifdef WITH_GUI
		if(err.what() == std::string{"mapfile: 1 argument(s) expected. 0 provided."}) {
			wxDISABLE_DEBUG_SUPPORT();

			// only pass program name
			int wx_argc = 1;
			return wxEntry(wx_argc, argv);
		}
#endif
		std::cerr << err.what() << "\n";
		// print usage message
		std::cerr << cli.usage() << "\n";
		std::exit(EXIT_FAILURE);
	}

	// generate image
	SDL_Surface *img = process(conf, cliErrorCallback);
	if (!img)
		std::exit(EXIT_FAILURE);

	// save image
	if (output.empty())
		output = conf.map.substr(0, conf.map.length() - 3) + "png";

	if (IMG_SavePNG(img, output.c_str()) < 0) {
		SDL_FreeSurface(img);
		cliErrorCallback(IMG_GetError());
		std::exit(EXIT_FAILURE);
	}
	SDL_FreeSurface(img);

	return EXIT_SUCCESS;
}

static SDL_Surface* process(ImgConfig conf, ErrorCallbackFunc error_cb, ErrorCallbackParam param) {
	if (!Exists(conf.map)) {
		error_cb("Input map file " + conf.map +" cannot be found.", param);
		return nullptr;
	}

	std::string path = GetFileDirectory(conf.map);
	CollectResourcePaths(path);

	if (conf.encoding.empty())
		conf.encoding = lcf::ReaderUtil::GetEncoding(path + "RPG_RT.ini");

	std::unique_ptr<lcf::rpg::Map> map(lcf::LMU_Reader::Load(conf.map, conf.encoding));
	if (!map) {
		error_cb(lcf::LcfReader::GetError(), param);
		return nullptr;
	}

	// ChipSet flags
	uint8_t csflag[65536] = {0};
	if (conf.chipset.empty()) {
		// Get chipset from database
		if (conf.database.empty())
			conf.database = path + "RPG_RT.ldb";

		auto db = lcf::LDB_Reader::Load(conf.database, conf.encoding);
		if (!db) {
			error_cb(lcf::LcfReader::GetError(), param);
			return nullptr;
		}

		assert(map->chipset_id <= static_cast<int>(db->chipsets.size()));
		lcf::rpg::Chipset & cs = db->chipsets[map->chipset_id - 1];
		std::string chipset_base(cs.chipset_name);

		conf.chipset = FindResource("ChipSet", chipset_base);
		if (conf.chipset.empty()) {
			error_cb("Chipset " + chipset_base + " cannot be found.", param);
			return nullptr;
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
		error_cb("Unable to create output image.", param);
		return nullptr;
	}

	if (!conf.no_events) {
		// Just do the Y-sort here. Yes, it modifies the data that's supposed to be rendered.
		// Doesn't particularly matter. What does matter is that this has to be a stable_sort,
		// so equivalent Y still causes ID order to be prioritized (just in case)
		std::stable_sort(map->events.begin(), map->events.end(),
			[](const auto& ev1, const auto& ev2) { return ev1.y < ev2.y; });
	}

	RenderCore(output_img, conf.chipset, csflag, map, conf);

	return output_img;
}

static void cliErrorCallback(const std::string& error, ErrorCallbackParam param) {
	// Simply tell about the error
	std::cerr << error << "\n";
}

#ifdef WITH_GUI
unsigned char *makeImage(ImgConfig conf, int &w, int &h, ErrorCallbackFunc error_cb,
	ErrorCallbackParam param) {

	// generate image
	SDL_Surface *img = process(conf, error_cb, param);
	if (!img)
		return nullptr;

	// create buffer
	unsigned char *img_data = new unsigned char[img->w * img->h * 4];
	if(!img_data) {
		return nullptr;
	}

	// copy for use in a wxBitmap
	w = img->w;
	h = img->h;
	memcpy(img_data, img->pixels, img->w * img->h * 4);

	SDL_FreeSurface(img);
	return img_data;
}
#endif
