/* chipset.h, types and prototypes for the map tileset management.
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
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CHIPSET_H
#define CHIPSET_H

#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>

constexpr int TILE_SIZE=16;
constexpr int HALF_TILE=TILE_SIZE/2;
constexpr int TILES_IN_ROW=32;
constexpr int TILES_IN_COL=45;
constexpr int CHIPSET_WIDTH=TILES_IN_ROW*TILE_SIZE;
constexpr int CHIPSET_HEIGHT=TILES_IN_COL*TILE_SIZE;

enum TILETYPE {
	UPPER = 0x2710,
	LOWER = 0x1388,
	TERRAIN = 0x0FA0,
	ANIMATED = 0x0BB8
};

// === Chipset structure ===================================================
struct Chipset {
	// --- Fields declaration ----------------------------------------------
	private:
		// The chipset structure holds the graphic tileset of a chipset, as well
		// as their properties and the methods for correctly displaying them.
		SDL_Surface * BaseSurface;    // Chipset's base surface!
		SDL_Surface * ChipsetSurface; // Chipset's precalculated surface

	// --- Methods declaration ---------------------------------------------
	public:
		Chipset() = delete;
		explicit Chipset(SDL_Surface * Surface);
		~Chipset();

		void RenderTile(SDL_Surface * dest, int tile_x, int tile_y, unsigned short Tile, int Frame);
		void RenderWaterTile(SDL_Surface * dest, unsigned short Tile, int Frame, int Border, int Water, int Combination);
		void RenderDepthTile(SDL_Surface * dest, unsigned short Tile, int Number, int Depth);
		void RenderTerrainTile(SDL_Surface * dest, unsigned short Tile, int Terrain, int Combination);
		void DrawSurface(SDL_Surface* destiny, int dX, int dY, int sX, int sY, int sW, int sH, bool fromBase = true);

	private:
		// Tile drawing helper functions
		void DrawFull(SDL_Surface *dest, int x, int y, int sX, int sY);
		void DrawQuarter(SDL_Surface *dest, int x, int y, int sX, int sY);
		void DrawWide(SDL_Surface *dest, int x, int y, int sX, int sY);
		void DrawTall(SDL_Surface *dest, int x, int y, int sX, int sY);
		void DrawEdges(SDL_Surface *dest, int x, int y, int sX, int sY, int Combination);
};

#endif
