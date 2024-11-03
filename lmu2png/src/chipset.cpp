/* chipset.cpp, routines for the map tileset management.
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

#include <stdlib.h>
#include <stdio.h>
#include "chipset.h"

#define BIT(x) (1 << (x))

// === Chipset structure ===================================================
Chipset::Chipset(FIBITMAP *Surface) {
	// Set base surface, used for generating the tileset
	m_Base.reset(FreeImage_Clone(Surface));
	m_Chipset.reset(FreeImage_Allocate(CHIPSET_WIDTH, CHIPSET_HEIGHT, 32));
	int CurrentTile = 0;

	// Generate water tiles A-C
	for (int type = 'a'; type<='c'; type++) {
		int border = 0;
		int water = 0;
		if (type == 'b')
			border = 1;
		if (type == 'c')
			water = 3;

		for (int frame = 0; frame<3; frame++) {
			for (int comb = 0; comb<47; comb++, CurrentTile++)
				RenderWaterTile(m_Chipset.get(), CurrentTile, frame, border, water, comb);
		}
	}

	// Generate water depth tiles
	for (int depth = 1; depth<4; depth+=2) {
		for (int i=0; i<48; i++, CurrentTile++)
			RenderDepthTile(m_Chipset.get(), CurrentTile, i, depth);
	}

	// Generate animated tiles
	for (int j=0; j<3; j++) {
		for (int i=0; i<4; i++, CurrentTile++) {
			int x = (CurrentTile%TILES_IN_ROW)*TILE_SIZE;
			int y = (CurrentTile/TILES_IN_ROW)*TILE_SIZE;
			int sX = 48+j*TILE_SIZE, sY = 64+i*TILE_SIZE;

			DrawFull(m_Chipset.get(), x, y, sX, sY);
		}
	}

	// Generate terrain tiles
	for (int terrain=0; terrain<12; terrain++) {
		for (int comb=0; comb<50; comb++, CurrentTile++)
			RenderTerrainTile(m_Chipset.get(), CurrentTile, terrain, comb);
	}

	// Generate common tiles
	for (int i=0; i<288; i++, CurrentTile++) {
		int x = (CurrentTile%TILES_IN_ROW)*TILE_SIZE;
		int y = (CurrentTile/TILES_IN_ROW)*TILE_SIZE;
		int sX = 192+((i%6)*TILE_SIZE)+(i/96)*96;
		int sY = ((i/6)%TILE_SIZE)*TILE_SIZE;

		DrawFull(m_Chipset.get(), x, y, sX, sY);
	}
}

Chipset::~Chipset() {
	//FreeImage_Save(FIF_PNG, m_Chipset.get(), "_chipset.png");
}

void Chipset::RenderTile(FIBITMAP *dest, int tile_x, int tile_y,
	unsigned short Tile, int Frame) {
	tile_x *= TILE_SIZE;
	tile_y *= TILE_SIZE;

	if (Tile >= TILETYPE::UPPER) {               // Upper layer tiles
		Tile = Tile - TILETYPE::UPPER + 0x04FB;
	} else if (Tile >= TILETYPE::LOWER) {        // Lower layer tiles
		Tile = Tile - TILETYPE::LOWER + 0x046B;
	} else if (Tile >= TILETYPE::TERRAIN) {      // Terrain tiles
		Tile = Tile - TILETYPE::TERRAIN + 0x0213;
	} else if (Tile >= TILETYPE::ANIMATED) {     // Animated tiles
		Frame %= 4;
		Tile = 0x0207 + (((Tile-TILETYPE::ANIMATED)/50)<<2) + Frame;
	} else {                                     // Water tiles
		Frame %= 3;
		int WaterTile = Tile%50;
		int WaterType = ((Tile/50)/20);
		Tile = WaterType*141+WaterTile+(Frame*47);
	}

	DrawSurface(dest, tile_x, tile_y, ((Tile&0x1F)<<4), ((Tile>>5)<<4), TILE_SIZE, TILE_SIZE, false);
}

void Chipset::RenderWaterTile(FIBITMAP *dest, unsigned short Tile, int Frame, int Border, int Water, int Combination) {
	int x = (Tile%TILES_IN_ROW)*TILE_SIZE;
	int y = (Tile/TILES_IN_ROW)*TILE_SIZE;
	int SFrame = Frame*16, SBorder = Border*48;
	int Sx = SFrame+SBorder;
	Combination &= 0x3F;

	// Since this function isn't meant to be used in realtime, we can allow
	// use nasty code here. First off, draw the water tile, for the background.
	DrawFull(dest, x, y,  SFrame, 64+(Water*16));

	// Now, get the combination from the tile and draw it using this stupidly hard coded routine.
	// I've found out that this was easier than just find out a damn algorithm.
	if (Combination & 0x20) {
		// This is where it gets nasty :S
		if (Combination > 0x29) {
			// Multiple edge possibilities
			switch(Combination) {
				case 0x2A:
					DrawWide(dest, x, y,           Sx, 0);
					DrawWide(dest, x, y+HALF_TILE, Sx, 24);
					break;
				case 0x2B:
					DrawTall(dest, x, y,           Sx,           0);
					DrawTall(dest, x+HALF_TILE, y, Sx+HALF_TILE, 32);
					break;
				case 0x2C:
					DrawWide(dest, x, y+HALF_TILE, Sx, 8);
					DrawWide(dest, x, y,           Sx, 16);
					break;
				case 0x2D:
					DrawTall(dest, x+HALF_TILE, y, Sx+HALF_TILE, 0);
					DrawTall(dest, x, y,           Sx,           32);
					break;
				case 0x2E:
					DrawFull(dest, x, y,  Sx, 0);
					break;
			}
		} else {
			// Wall + inner edges
			switch((Combination>>1)&0x07) {
				case 0x00:
					DrawFull(dest, x, y, Sx, (Combination&BIT(0)) ? 32 : 16);
					break;
				case 0x01:
					DrawQuarter(dest, x,           y,           Sx,           0);  // Corner
					DrawQuarter(dest, x,           y+HALF_TILE, Sx,           24); // Left/Right frame
					DrawQuarter(dest, x+HALF_TILE, y,           Sx+HALF_TILE, 32); // Top/Bottom frame

					if (Combination&BIT(0))
						DrawQuarter(dest, x+HALF_TILE, y+HALF_TILE, Sx+HALF_TILE, 48+HALF_TILE);
					break;
				case 0x02:
					DrawQuarter(dest, x+HALF_TILE, y,           Sx+HALF_TILE, 0);  // Corner
					DrawQuarter(dest, x+HALF_TILE, y+HALF_TILE, Sx+HALF_TILE, 24); // Left/Right frame
					DrawQuarter(dest, x,           y,           Sx,           32); // Top/Bottom frame

					if (Combination&BIT(0))
						DrawQuarter(dest, x, y+HALF_TILE, Sx, 48+HALF_TILE);
					break;
				case 0x03:
					DrawQuarter(dest, x+HALF_TILE, y+HALF_TILE, Sx+HALF_TILE, HALF_TILE); // Corner
					DrawQuarter(dest, x+HALF_TILE, y,           Sx+HALF_TILE, TILE_SIZE); // Left/Right frame
					DrawQuarter(dest, x,           y+HALF_TILE, Sx,           40);        // Top/Bottom frame

					if (Combination&BIT(0))
						DrawQuarter(dest, x, y, Sx, 48);
					break;
				case 0x04:
					DrawQuarter(dest, x,           y+HALF_TILE, Sx,           HALF_TILE); // Corner
					DrawQuarter(dest, x,           y,           Sx,           TILE_SIZE); // Left/Right frame
					DrawQuarter(dest, x+HALF_TILE, y+HALF_TILE, Sx+HALF_TILE, 40);        // Top/Bottom frame

					if (Combination&BIT(0))
						DrawQuarter(dest, x+HALF_TILE, y, Sx+HALF_TILE, 48);
					break;
			}
		}
	} else if (Combination & 0x10) {
		// Wall + inner edge cases. They're also easier to find out the
		// values here too
		switch((Combination>>2)&0x03) {
			case 0x00:
				// Render left wall
				DrawTall(dest, x, y, Sx, 16);

				// Render top right corner and bottom right corner
				if (Combination&BIT(0))
					DrawQuarter(dest, x+HALF_TILE, y,           Sx+HALF_TILE, 48);
				if (Combination&BIT(1))
					DrawQuarter(dest, x+HALF_TILE, y+HALF_TILE, Sx+HALF_TILE, 48+HALF_TILE);
				break;
			case 0x01:
				// Render top wall
				DrawWide(dest, x, y, Sx, 32);

				// Render bottom right corner and bottom left corner
				if (Combination&BIT(0))
					DrawQuarter(dest, x+HALF_TILE, y+HALF_TILE, Sx+HALF_TILE, 48+HALF_TILE);
				if (Combination&BIT(1))
					DrawQuarter(dest, x,           y+HALF_TILE, Sx,           48+HALF_TILE);
				break;
			case 0x02:
				// Right wall
				DrawTall(dest, x+HALF_TILE, y, Sx+HALF_TILE, 16);

				// Render bottom left corner and top left corner
				if (Combination&BIT(0))
					DrawQuarter(dest, x, y+HALF_TILE, Sx, 48+HALF_TILE);
				if (Combination&BIT(1))
					DrawQuarter(dest, x, y,           Sx, 48);
				break;
			case 0x03:
				// Bottom wall
				DrawWide(dest, x, y+HALF_TILE, Sx, 32+HALF_TILE);

				// Render top left corner and top right corner
				if (Combination&BIT(0))
					DrawQuarter(dest, x,           y, Sx,           48);
				if (Combination&BIT(1))
					DrawQuarter(dest, x+HALF_TILE, y, Sx+HALF_TILE, 48);
				break;
		}
	} else {
		// Single inner edge cases.
		DrawEdges(dest, x, y, Sx, 48, Combination);
	}
}

void Chipset::RenderTerrainTile(FIBITMAP *dest, unsigned short Tile, int Terrain, int Combination) {
	int x = (Tile%TILES_IN_ROW)*TILE_SIZE;
	int y = (Tile/TILES_IN_ROW)*TILE_SIZE;
	Terrain += 4;
	int sX = ((Terrain%2)*48)+(Terrain/8)*96, sY = ((Terrain/2)%4)*64;

	// Since this function isn't meant to be used in realtime, we can allow
	// use nasty code here. First off, draw the water tile, for the background.
	DrawFull(dest, x, y, sX+16, sY+32);

	// Now, get the combination from the tile and draw it using this stupidly
	// hard coded routine. I've found out that this was easier than just find
	// out a damn algorithm.
	if (Combination & 0x20) {
		// This is where it gets nasty :S
		if (Combination > 0x29) {
			// Multiple edge possibilities
			switch(Combination) {
				case 0x2A:
					DrawTall(dest, x, y,           sX,    sY+16);
					DrawTall(dest, x+HALF_TILE, y, sX+40, sY+16);
					break;
				case 0x2B:
					DrawWide(dest, x, y,           sX,    sY+16);
					DrawWide(dest, x, y+HALF_TILE, sX,    sY+56);
					break;
				case 0x2C:
					DrawTall(dest, x, y,           sX,    sY+48);
					DrawTall(dest, x+HALF_TILE, y, sX+40, sY+48);
					break;
				case 0x2D:
					DrawWide(dest, x, y,           sX+32, sY+16);
					DrawWide(dest, x, y+HALF_TILE, sX+32, sY+56);
					break;
				case 0x2E:
					DrawQuarter(dest, x,           y,           sX,    sY+16);
					DrawQuarter(dest, x+HALF_TILE, y,           sX+40, sY+16);
					DrawQuarter(dest, x,           y+HALF_TILE, sX,    sY+56);
					DrawQuarter(dest, x+HALF_TILE, y+HALF_TILE, sX+40, sY+56);
					break;
				case 0x31:
					DrawFull(dest, x, y, sX, sY);
					break;
			}
		} else {
			// Wall + inner edges
			switch((Combination>>1)&0x07) {
				case 0x00:
					if (Combination&BIT(0)) {
						DrawWide(dest, x, y,           sX+16, sY+16);
						DrawWide(dest, x, y+HALF_TILE, sX+16, sY+56);
					} else {
						DrawTall(dest, x,           y, sX,    sY+32);
						DrawTall(dest, x+HALF_TILE, y, sX+32, sY+32);
					}
					break;
				case 0x01:
					DrawFull(dest, x, y,  sX, sY+16);
					if (Combination&BIT(0))
						DrawQuarter(dest, x+HALF_TILE, y+HALF_TILE, sX+40, sY+HALF_TILE);
					break;
				case 0x02:
					DrawFull(dest, x, y,  sX+32, sY+16);
					if (Combination&BIT(0))
						DrawQuarter(dest, x, y+HALF_TILE, sX+32, sY+HALF_TILE);
					break;
				case 0x03:
					DrawFull(dest, x, y,  sX+32, sY+48);
					if (Combination&BIT(0))
						DrawQuarter(dest, x, y, sX+32, sY);
					break;
				case 0x04:
					DrawFull(dest, x, y,  sX, sY+48);
					if (Combination&BIT(0))
						DrawQuarter(dest, x+HALF_TILE, y, sX+40, sY);
					break;
			}
		}
	} else if (Combination & 0x10) {
		// Wall + inner edge cases. They're also easier to find out the values here too
		switch((Combination>>2)&0x03) {
			case 0x00:
				// Render left wall
				DrawFull(dest, x, y,  sX, sY+32);

				// Render top right corner and bottom right corner
				if (Combination&BIT(0))
					DrawQuarter(dest, x+HALF_TILE, y,           sX+40, sY);
				if (Combination&BIT(1))
					DrawQuarter(dest, x+HALF_TILE, y+HALF_TILE, sX+40, sY+HALF_TILE);
				break;
			case 0x01:
				// Render top wall
				DrawFull(dest, x, y,  sX+16, sY+16);

				// Render bottom right corner and bottom left corner
				if (Combination&BIT(0))
					DrawQuarter(dest, x+HALF_TILE, y+HALF_TILE, sX+40, sY+HALF_TILE);
				if (Combination&BIT(1))
					DrawQuarter(dest, x,           y+HALF_TILE, sX+32, sY+HALF_TILE);
				break;
			case 0x02:
				// Right wall
				DrawFull(dest, x, y,  sX+32, sY+32);

				// Render bottom left corner and top left corner
				if (Combination&BIT(0))
					DrawQuarter(dest, x, y+HALF_TILE, sX+32, sY+HALF_TILE);
				if (Combination&BIT(1))
					DrawQuarter(dest, x, y,           sX+32, sY);
				break;
			case 0x03:
				// Bottom wall
				DrawFull(dest, x, y,  sX+16, sY+48);

				// Render top left corner and top right corner
				if (Combination&BIT(0))
					DrawQuarter(dest, x,           y, sX+32, sY);
				if (Combination&BIT(1))
					DrawQuarter(dest, x+HALF_TILE, y, sX+40, sY);
				break;
		}
	} else {
		// Single inner edge cases.
		DrawEdges(dest, x, y, sX+32, sY, Combination);
	}
}

void Chipset::RenderDepthTile(FIBITMAP *dest, unsigned short Tile, int Number, int Depth) {
	int x = (Tile%TILES_IN_ROW)*TILE_SIZE;
	int y = (Tile/TILES_IN_ROW)*TILE_SIZE;
	int Frame = Number/16;
	int DepthCombination = Number%16;
	int sX = Frame*16;
	int sY = 64+(Depth*16);

	// Depth part edges
	DrawEdges(dest, x, y, sX, sY, DepthCombination);
}

void Chipset::DrawSurface(FIBITMAP *dest, int dX, int dY, int sX, int sY, int sW, int sH, bool fromBase) {
	FIBITMAP* src = fromBase ? m_Base.get() : m_Chipset.get();

	if (sW == -1)
		sW = FreeImage_GetWidth(src);
	if (sH == -1)
		sH = FreeImage_GetHeight(src);

	CustomAlphaCombine(src, sX, sY, dest, dX, dY, sW, sH);
}

inline void Chipset::DrawFull(FIBITMAP *dest, int x, int y, int sX, int sY) {
	DrawSurface(dest, x, y, sX, sY, TILE_SIZE, TILE_SIZE);
}

inline void Chipset::DrawQuarter(FIBITMAP *dest, int x, int y, int sX, int sY) {
	DrawSurface(dest, x, y, sX, sY, HALF_TILE, HALF_TILE);
}

inline void Chipset::DrawWide(FIBITMAP *dest, int x, int y, int sX, int sY) {
	DrawSurface(dest, x, y, sX, sY, TILE_SIZE, HALF_TILE);
}

inline void Chipset::DrawTall(FIBITMAP *dest, int x, int y, int sX, int sY) {
	DrawSurface(dest, x, y, sX, sY, HALF_TILE, TILE_SIZE);
}

inline void Chipset::DrawEdges(FIBITMAP *dest, int x, int y, int sX, int sY, int Combination) {
	if (Combination&BIT(0))
		DrawQuarter(dest, x,           y,           sX,           sY);           // top left
	if (Combination&BIT(1))
		DrawQuarter(dest, x+HALF_TILE, y,           sX+HALF_TILE, sY);           // top right
	if (Combination&BIT(2))
		DrawQuarter(dest, x+HALF_TILE, y+HALF_TILE, sX+HALF_TILE, sY+HALF_TILE); // bottom right
	if (Combination&BIT(3))
		DrawQuarter(dest, x,           y+HALF_TILE, sX,           sY+HALF_TILE); // bottom left
}
