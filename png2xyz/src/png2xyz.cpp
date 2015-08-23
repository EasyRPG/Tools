/*
 * This file is part of png2xyz. Copyright (c) 2015 png2xyz authors.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <zlib.h>
#include <png.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#ifdef _WIN32
# include <algorithm>
#endif

# ifdef __MINGW64_VERSION_MAJOR
int _dowildcard = -1; /* enable wildcard expansion for mingw-w64 */
# endif

/** Returns the filename (without extension). */
std::string GetFilename(const std::string& str);

/** Returns that path (everything left to the last /) */
std::string GetPath(const std::string& str);

std::string GetFilename(const std::string& str) {
	std::string s = str;
#ifdef _WIN32
	std::replace(s.begin(), s.end(), '\\', '/');
#endif

	// Extension
	size_t found = s.find_last_of(".");
	if(found != std::string::npos)
	{
		s = s.substr(0, found);
	}

	// Filename
	found = s.find_last_of("/");
	if(found == std::string::npos)
	{
		return s;
	}

	s = s.substr(found + 1);
	return s;
}

std::string GetPath(const std::string& str) {
	std::string s = str;
#ifdef _WIN32
	std::replace(s.begin(), s.end(), '\\', '/');
#endif
	// Path
	size_t found = s.find_last_of("/");
	if(found == std::string::npos)
	{
		return ".";
	}

	s = s.substr(0, found);
	return s;
}

int main(int argc, char* argv[]) {
	if(argc < 2)
	{
		std::cout << "Usage: " << argv[0]
			<< " filename" << std::endl;
		return 1;
	}

	for(int arg = 1; arg < argc; arg++) {
		FILE *png_file;
		unsigned char* header;
		png_structp png_ptr;
		png_infop info_ptr;
		unsigned short width;
		unsigned short height;
		unsigned int bit_depth;
		unsigned int color_type;
		png_colorp palette;
		int num_palette;
		png_bytep *row_pointers;
		Bytef* xyz_data;
		uLong comp_size;
		Bytef* comp_data;
		std::string xyz_filename;

		// Open PNG file
		png_file = fopen(argv[arg], "rb");
		if(png_file == NULL) {
			std::cerr << "Error reading file "
				<< argv[arg] << "." << std::endl;
			return 1;
		}

		// Read PNG file header
		header = new unsigned char[8];
		if (fread(header, 1, 8, png_file) != 8) {
			std::cerr << "Error reading PNG header of file "
				<< argv[arg] << "." << std::endl;
			delete[] header;
			return 1;
		}

		// Check PNG validity
		if(png_sig_cmp(header, 0, 8) != 0) {
			std::cerr << "Input file " << argv[arg]
				<< " is not a PNG file." << std::endl;
			delete[] header;
			return 1;
		}
		delete[] header;

		// Create PNG read structure
		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
			NULL, NULL);
		if(png_ptr == NULL)
		{
			std::cerr << "Error creating PNG read structure for "
				<< argv[arg] << "." << std::endl;
			fclose(png_file);
			return 1;
		}

		// Create PNG info structure
		info_ptr = png_create_info_struct(png_ptr);
		if(info_ptr == NULL)
		{
			std::cerr << "Error creating PNG info structure for "
				<< argv[arg] << "." << std::endl;
			png_destroy_read_struct(&png_ptr, NULL, NULL);
			fclose(png_file);
			return 1;
		}

		// Init I/O functions
		if(setjmp(png_jmpbuf(png_ptr)))
		{
			std::cerr << "Error initializing PNG I/O for "
				<< argv[arg] << "." << std::endl;
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			fclose(png_file);
			return 1;
		}
		png_init_io(png_ptr, png_file);

		// Already read 8 header bytes, let libpng know about this
		png_set_sig_bytes(png_ptr, 8);

		// Read PNG
		png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

		// Check PNG dimensions
		width = png_get_image_width(png_ptr, info_ptr);
		height = png_get_image_height(png_ptr, info_ptr);

		// Check bit depth validity
		bit_depth = png_get_bit_depth(png_ptr, info_ptr);
		if(bit_depth != 8) {
			std::cerr << "PNG file " << argv[arg]
				<< " is not using 8 bit depth." << std::endl;
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			fclose(png_file);
			return 1;
		}

		// Check color type validity
		color_type = png_get_color_type(png_ptr, info_ptr);
		if(color_type != PNG_COLOR_TYPE_PALETTE) {
			std::cerr << "PNG file " << argv[arg]
				<< " is not palette based." << std::endl;
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			fclose(png_file);
			return 1;
		}

		// Check palette chunk validity
		if(png_get_valid(png_ptr, info_ptr, PNG_INFO_PLTE) == 0) {
			std::cerr << "PNG file " << argv[arg]
				<< " has an invalid palette chunk."
				<< std::endl;
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			fclose(png_file);
			return 1;
		}

		// Get palette and color count
		png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);

		// Check palette color count validity
		if(num_palette != 256) {
			std::cerr << "PNG file " << argv[arg]
				<< " has lesser than 256 colors in palette."
				<< std::endl;
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			fclose(png_file);
			return 1;
		}

		xyz_data = new unsigned char[768 + width * height];

		// Create XYZ palette
		for (size_t i = 0; i < 256; i++) {
			xyz_data[i * 3] = palette[i].red;
			xyz_data[i * 3 + 1] = palette[i].green;
			xyz_data[i * 3 + 2] = palette[i].blue;
		}

		// Get image rows
		row_pointers = png_get_rows(png_ptr, info_ptr);

		// Create XYZ image
		for (size_t y = 0; y < height; y++) {
			memcpy(&xyz_data[768 + y * width],
			row_pointers[y], width);
		}

		// Close PNG file
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(png_file);

		// Compress XYZ data
		comp_size = compressBound(768 + width * height);
		comp_data = new Bytef[comp_size];

		int errorcode = compress2(comp_data, &comp_size, xyz_data,
			768 + width * height, Z_BEST_COMPRESSION);
		delete[] xyz_data;
		if(errorcode != Z_OK) {
			std::cerr << "Error while compressing XYZ data from "
				<< argv[arg] << "." << std::endl;
			delete[] comp_data;
			return 1;
		}

		xyz_filename = GetFilename(argv[arg]) + ".xyz";
		std::ofstream xyz_file(xyz_filename.c_str(), std::ofstream::binary);
		xyz_file.write("XYZ1", 4);
		xyz_file.write(reinterpret_cast<char*>(&width), 2);
		xyz_file.write(reinterpret_cast<char*>(&height), 2);
		xyz_file.write(reinterpret_cast<char*>(comp_data), comp_size);
		xyz_file.close();
		delete[] comp_data;
	}

	return 0;
}
