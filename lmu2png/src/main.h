/* main.h
   Copyright (C) 2024 EasyRPG Project <https://github.com/EasyRPG/>.

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

#ifndef MAIN_H
#define MAIN_H

#include <FreeImage.h>
#include <memory>

struct FIBITMAPDeleter {
        void operator()(FIBITMAP* dib) {
                FreeImage_Unload(dib);
        }
};
using BitmapPtr = std::unique_ptr<FIBITMAP, FIBITMAPDeleter>;

void CustomAlphaCombine(FIBITMAP *src, int sLeft, int sTop, FIBITMAP *dst, int dLeft, int dTop, int width, int height);

#endif
