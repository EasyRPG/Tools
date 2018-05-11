/* sdlxyz.h, SDL image loader for XYZ files.
   Copyright (C) 2018 EasyRPG Project <https://github.com/EasyRPG/>.

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

#include "sdlxyz.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <zlib.h>

static SDL_Surface* XYZLoaderMakeSurface(int w, int h, unsigned char * data) {
	SDL_Surface* sf = SDL_CreateRGBSurface(0, w, h, 8, 0, 0, 0, 0);
	if (sf) {
		SDL_Palette * palette = sf->format->palette;
		// According to SDL_Surface's remarks,
		//  no lock is needed unless RLE-optimized,
		//  so we can just avoid that potential error case.
		for (int i = 0; i < 256; i++) {
			palette->colors[i].r = data[(i * 3) + 0];
			palette->colors[i].g = data[(i * 3) + 1];
			palette->colors[i].b = data[(i * 3) + 2];
			palette->colors[i].a = 255;
		}
		memcpy(sf->pixels, data + 768, w * h);
	}
	return sf;
}

static SDL_Surface* XYZLoaderCore(FILE* f) {
	fseek(f, 0, SEEK_END);
	long compressedDataSize = ftell(f) - 8;
	fseek(f, 0, SEEK_SET);
	// Read header
	if (fgetc(f) != 'X')
		return NULL;
	if (fgetc(f) != 'Y')
		return NULL;
	if (fgetc(f) != 'Z')
		return NULL;
	if (fgetc(f) != '1')
		return NULL;
	int tmp, w, h;
	tmp = fgetc(f);
	if (tmp < 0)
		return NULL;
	w = tmp;
	tmp = fgetc(f);
	if (tmp < 0)
		return NULL;
	w |= tmp << 8;
	tmp = fgetc(f);
	if (tmp < 0)
		return NULL;
	h = tmp;
	tmp = fgetc(f);
	if (tmp < 0)
		return NULL;
	h |= tmp << 8;
	size_t total = 768 + (w * h);
	size_t totalBackup = total;
	// Now the room for error has mostly ceded, allocate & fill buffers
	unsigned char * compressedData = (unsigned char *) malloc(compressedDataSize);
	if (!compressedData)
		return NULL;
	if (fread(compressedData, compressedDataSize, 1, f) != 1) {
		free(compressedData);
		return NULL;
	}
	unsigned char * decompressedData = (unsigned char *) malloc(total);
	if (!decompressedData) {
		free(compressedData);
		return NULL;
	}
	int status = uncompress(decompressedData, &total, compressedData, compressedDataSize);
	SDL_Surface * sf = NULL;
	if (totalBackup != total)
		status = Z_STREAM_END;
	if (status == Z_OK)
		sf = XYZLoaderMakeSurface(w, h, decompressedData);
	free(compressedData);
	free(decompressedData);
	return sf;
}

SDL_Surface* LoadImageXYZ(const char* image_path) {
	FILE* f = fopen(image_path, "rb");
	if (!f)
		return NULL;
	// XYZ
	SDL_Surface* sf = XYZLoaderCore(f);
	fclose(f);
	return sf;
}

