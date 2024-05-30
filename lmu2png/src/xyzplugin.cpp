/* xyzplugin.cpp
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

#include "xyzplugin.h"
#include <cstring>
#include <cstdio>
#include <zlib.h>

// for internal use

static int s_format_id;

// description of the plugin format, etc.

static const char *Format() {
	return "XYZ";
}

static const char *Description() {
	return "RPGMaker 2000/2003 Image";
}

static const char *Extension() {
	return "xyz";
}

static const char *MimeType() {
	return "image/x-xyz";
}

static BOOL SupportsExportDepth(int /* depth */) {
	return FALSE;
}

static BOOL SupportsExportType(FREE_IMAGE_TYPE /* type */) {
	return FALSE;
}

static BOOL SupportsICCProfiles() {
	return FALSE;
}

// determine if we are dealing with XYZ

static int Validate(FreeImageIO *io, fi_handle handle) {
	BYTE xyz_signature[4] = { 'X', 'Y', 'Z', '1' };
	BYTE signature[4] = { 0, 0, 0, 0 };

	io->read_proc(&signature, 1, sizeof(xyz_signature), handle);

	return (memcmp(xyz_signature, signature, sizeof(xyz_signature)) == 0);
}

#ifdef FREEIMAGE_BIGENDIAN
static inline void SwapShort(unsigned short *sp) {
	unsigned char *cp = (unsigned char *)sp, t = cp[0];
	cp[0] = cp[1];
	cp[1] = t;
}
#endif

static inline unsigned short ReadShort(FreeImageIO *io, fi_handle handle) {
	unsigned short level = 0;
	io->read_proc(&level, 2, 1, handle);
#ifdef FREEIMAGE_BIGENDIAN
	SwapShort(&level);
#endif
	return level;
}

// load image

static FIBITMAP *Load(FreeImageIO *io, fi_handle handle, int /* page */, int /* flags */, void */* data */) {
	FIBITMAP *dib = nullptr;
	Bytef *compressedData = nullptr, *decompressedData = nullptr;
	constexpr int paletteEntries = 256;
	constexpr int paletteSize = paletteEntries * 3;

	if (!handle)
		return nullptr;

	try {
		// skip (already validated) magic
		io->seek_proc(handle, 4, SEEK_CUR);

		// get dimensions
		int width = ReadShort(io, handle);
		int height = ReadShort(io, handle);

		// create a dib and write the bitmap header
		dib = FreeImage_Allocate(width, height, 8);
		if(!dib) {
			throw "Failed to allocate memory for BITMAP.";
		}

		// get file size without header
		io->seek_proc(handle, 0, SEEK_END);
		long compressedDataSize = io->tell_proc(handle) - 8;
		io->seek_proc(handle, 8, SEEK_SET);

		uLongf total = paletteSize + (width * height);
		uLongf totalBackup = total;

		// Now the room for error has mostly ceded, allocate & fill buffers
		compressedData = new Bytef[compressedDataSize];
		if (!compressedData)
			throw "Failed to allocate memory for file buffer.";

		io->read_proc(compressedData, 1, compressedDataSize, handle);

		decompressedData = new Bytef[total];
		if (!decompressedData)
			throw "Failed to allocate memory for decompression buffer.";

		int status = uncompress(decompressedData, &total, compressedData, compressedDataSize);

		if (totalBackup != total)
			throw "Failed to uncompress image.";

		if (status == Z_OK) {
			// store the palette
			RGBQUAD *palette = FreeImage_GetPalette(dib);
			for(int i = 0; i < paletteEntries; i++) {
				palette[i].rgbRed   = decompressedData[(i * 3) + 0];
				palette[i].rgbGreen = decompressedData[(i * 3) + 1];
				palette[i].rgbBlue  = decompressedData[(i * 3) + 2];
			}

			// copy in the bitmap bits via the pointer table
			const BYTE* line = decompressedData + paletteSize;
			for (int y = 0; y < height; y++) {
				BYTE *dst_line = FreeImage_GetScanLine(dib, height - 1 - y);
				memcpy(dst_line, line, width);

				// advance to next
				line += width;
			}
		}

		// cleanup
		delete[] compressedData;
		delete[] decompressedData;

		return dib;

	} catch (const char *text) {
		// free zlib buffers
		if(compressedData)
			delete[] compressedData;
		if(decompressedData)
			delete[] decompressedData;

		// free bitmap struct
		if (dib) {
			FreeImage_Unload(dib);
		}

		// notify error
		if (nullptr != text) {
			FreeImage_OutputMessageProc(s_format_id, text);
		}

		return nullptr;
	}
}

// finally describe the plugin

void InitXYZ(Plugin *plugin, int format_id) {
	s_format_id = format_id;

	plugin->format_proc = Format;
	plugin->description_proc = Description;
	plugin->extension_proc = Extension;
	plugin->regexpr_proc = nullptr;
	plugin->open_proc = nullptr;
	plugin->close_proc = nullptr;
	plugin->pagecount_proc = nullptr;
	plugin->pagecapability_proc = nullptr;
	plugin->load_proc = Load;
	plugin->save_proc = nullptr;
	plugin->validate_proc = Validate;
	plugin->mime_proc = MimeType;
	plugin->supports_export_bpp_proc = SupportsExportDepth;
	plugin->supports_export_type_proc = SupportsExportType;
	plugin->supports_icc_profiles_proc = SupportsICCProfiles;
}
