/*
 * This file is part of xml2png. Copyright (c) 2015 xyz2png authors.
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
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>

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
		std::ifstream file(argv[arg],
			std::ios::binary | std::ios::ate);
		if(!file) {
			std::cerr << "Error reading file "
				<< argv[arg] << "." << std::endl;
			return 1;
		}

		long size = file.tellg();
		char* header = new char[4];

		file.seekg(0, std::ios::beg);
		file.read((char*) header, 4);
		if(memcmp(header, "XYZ1", 4) != 0) {
			std::cerr << "Input file " << argv[arg]
				<< " is not a XYZ file: '"
				<< header << "'." << std::endl;
			delete[] header;
			return 1;
		}
		delete[] header;

		unsigned short width;
		unsigned short height;
		file.read((char*) &width, 2);
		file.read((char*) &height, 2);

		int compressed_xyz_size = size - 8;
		Bytef* compressed_xyz_data = new Bytef[compressed_xyz_size];

		file.read((char*) compressed_xyz_data, compressed_xyz_size);

		uLongf xyz_size = 768 + (width * height);
		std::vector<Bytef> xyz_data(
			xyz_size);

		int status = uncompress(&xyz_data.front(),
			&xyz_size, compressed_xyz_data,
			compressed_xyz_size);

		if(status != Z_OK) {
			std::cerr << "Error uncompressing XYZ file "
				<< argv[arg] << "." << std::endl;
			return 1;
		}

		FILE *png_file;
		png_structp png_ptr;
		png_infop info_ptr;
		std::string png_filename;

		png_filename = GetFilename(argv[arg]) + ".png";

		// Open file for writing
		png_file = fopen(png_filename.c_str(), "wb");
		if(png_file == NULL) {
			std::cerr << "Error creating file "
				<< png_filename<< "." << std::endl;
			return 1;
		}

		// Create PNG write structure
		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,
			NULL, NULL);
		if(png_ptr == NULL)
		{
			std::cerr << "Error creating PNG write structure for "
				<< png_filename << "." << std::endl;
			fclose(png_file);
			return 1;
		}

		// Create PNG info structure
		info_ptr = png_create_info_struct(png_ptr);
		if(info_ptr == NULL)
		{
			std::cerr << "Error creating PNG info structure for "
				<< png_filename << "." << std::endl;
			fclose(png_file);
			png_destroy_write_struct(&png_ptr, NULL);
			return 1;
		}

		// Init I/O functions
		if(setjmp(png_jmpbuf(png_ptr)))
		{
			std::cerr << "Error initializing PNG I/O for "
				<< png_filename << "." << std::endl;
			fclose(png_file);
			png_destroy_write_struct(&png_ptr, &info_ptr);
			return 1;
		}
		png_init_io(png_ptr, png_file);

		// Set compression parameters
		png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);
		png_set_compression_mem_level(png_ptr, MAX_MEM_LEVEL);
		png_set_compression_buffer_size(png_ptr, 1024 * 1024);

		// Write header
		if(setjmp(png_jmpbuf(png_ptr))) {
			std::cerr << "Error writing PNG header for "
				<< png_filename << "." << std::endl;
			fclose(png_file);
			png_destroy_write_struct(&png_ptr, &info_ptr);
			return 1;
		}
		png_set_IHDR(png_ptr, info_ptr, width, height, 8,
			PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

		// Write palette
		if(setjmp(png_jmpbuf(png_ptr))) {
			std::cerr << "Error writing PNG palette for "
				<< png_filename << "." << std::endl;
			fclose(png_file);
			png_destroy_write_struct(&png_ptr, &info_ptr);
			return 1;
		}
		png_colorp palette = (png_colorp) png_malloc(png_ptr,
			PNG_MAX_PALETTE_LENGTH * (sizeof (png_color)));

	 	for(int i = 0; i < PNG_MAX_PALETTE_LENGTH; i++)
		{
			palette[i].red = xyz_data[i * 3];
			palette[i].green = xyz_data[i * 3 + 1];
			palette[i].blue = xyz_data[i * 3 + 2];
		}
		png_set_PLTE(png_ptr, info_ptr, palette,
			PNG_MAX_PALETTE_LENGTH);

		png_write_info(png_ptr, info_ptr);

		png_bytep* row_pointers = new png_bytep[height];
		for(int i = 0; i < height; i++) {
			row_pointers[i] =
				&xyz_data[768 + width * i];
		}
		png_write_image(png_ptr, row_pointers);
		delete[] row_pointers;
	
		png_write_end(png_ptr, info_ptr);

		png_free(png_ptr, palette);
		palette = NULL;

		png_destroy_write_struct(&png_ptr, &info_ptr);

		fclose(png_file);
	}

	return 0;
}
