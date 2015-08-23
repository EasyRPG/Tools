/* main.cpp, lmu2png main file.
   Copyright (C) 2015 EasyRPG Project <https://github.com/EasyRPG/>.

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

// *****************************************************************************
// =============================================================================
    #include <iostream>
    #include "SDL_image.h"
    #include "reader_lcf.h"
    #include "lmu_reader.h"
    #include "rpg_map.h"
    #include "chipset.h"
// =============================================================================
// *****************************************************************************

    // =========================================================================
    int main(int argc, char** argv)
    {
        const char * usage = "Usage: lmu2png map.lmu chipset.png output.png\n";

        if(argc < 4 || argc > 5)
        {
            std::cout<<usage;
            exit(EXIT_FAILURE);
        }

        const char* map_path = argv[1];
        const char* chipset_path = argv[2];
        const char* output_path = argv[3];

        std::auto_ptr<RPG::Map> map = LMU_Reader::Load(map_path, "");
        if (map.get() == NULL)
        {
            std::cerr<<LcfReader::GetError()<<std::endl;
            exit(EXIT_FAILURE);
        }

        SDL_Surface* chipset = IMG_Load(chipset_path);
        if (chipset == NULL)
        {
            std::cerr<<IMG_GetError()<<std::endl;
            exit(EXIT_FAILURE);
        }

        if (IMG_isBMP(SDL_RWFromFile(chipset_path, "rb")))
        {
            // Set as color key the first color in the palette
            SDL_Color ckey = chipset->format->palette->colors[0];
            SDL_SetColorKey(chipset, SDL_TRUE, SDL_MapRGB(chipset->format, ckey.r, ckey.g, ckey.b));
        }

        SDL_Surface* output = SDL_CreateRGBSurface(0, map->width * 16, map->height * 16, 8, 0, 0, 0, 0);
        output->format->palette = chipset->format->palette;
        stChipset gen;
        gen.GenerateFromSurface(chipset);

        for (int y = 0; y < map->height; ++y)
            for (int x = 0; x < map->width; ++x)
            {
                gen.RenderTile(output, x*16, y*16, map->lower_layer[x+y*map->width], 0);
                gen.RenderTile(output, x*16, y*16, map->upper_layer[x+y*map->width], 0);
            }

        std::vector<RPG::Event>::iterator ev;
        for (ev = map->events.begin(); ev != map->events.end(); ++ev)
        {
            RPG::EventPage evp = ev->pages[0];
            if (evp.character_name.empty())
                gen.RenderTile(output, (ev->x)*16, (ev->y)*16, 0x2710 + evp.character_index, 0);
        }

        if(IMG_SavePNG(output, output_path) < 0)
        {
	    std::cerr<<IMG_GetError()<<std::endl;
	    exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);
}
