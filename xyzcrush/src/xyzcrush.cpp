/*
 * This file is part of xyzcrush. Copyright (c) 2017 xyzcrush authors.
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
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#ifdef _WIN32
# include <algorithm>
#endif
#include "zopfli/zlib_container.h"

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
	if(found != std::string::npos) {
		s = s.substr(0, found);
	}

	// Filename
	found = s.find_last_of("/");
	if(found == std::string::npos) {
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
	if(found == std::string::npos) {
		return ".";
	}

	s = s.substr(0, found);
	return s;
}

int main(int argc, char* argv[]) {
	ZopfliOptions zopfli_options;
	ZopfliInitOptions(&zopfli_options);
	zopfli_options.verbose = 0;
	zopfli_options.verbose_more = 0;
	zopfli_options.numiterations = 15;
	zopfli_options.blocksplitting = 1;
	zopfli_options.blocksplittinglast = 0;
	zopfli_options.blocksplittingmax = 15;

	if (argc < 2) {
		std::cout << "Usage: " << argv[0]
			<< " filename" << std::endl;
		return 1;
	}

	for (int arg = 1; arg < argc; arg++) {
		std::ifstream file(argv[arg],
			std::ios::binary | std::ios::ate);
		if (!file) {
			std::cerr << "Error reading file " << argv[arg] << "."
				<< std::endl;
			return 1;
		}

		long size = file.tellg();
		char* header = new char[4];

		file.seekg(0, std::ios::beg);
		file.read((char*) header, 4);
		if (memcmp(header, "XYZ1", 4) != 0) {
			std::cerr << "Input file " << argv[arg]
				<< " is not a XYZ file: '" << header << "'."
				<< std::endl;
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
		Bytef* xyz_data = new Bytef[xyz_size];

		int status = uncompress(xyz_data, &xyz_size,
			compressed_xyz_data, compressed_xyz_size);

		if (status != Z_OK) {
			std::cerr << "XYZ error in file " << argv[arg] << "."
				<< std::endl;
			return 1;
		}
		delete[] compressed_xyz_data;

		// Compress XYZ data
		size_t comp_size = 0;
		unsigned char* comp_data = 0;

		ZopfliZlibCompress(&zopfli_options, xyz_data, xyz_size,
			&comp_data, &comp_size);

		delete[] xyz_data;

		std::stringstream ss;
		ss << GetFilename(argv[arg]) + std::string(".xyz");
		std::string xyz_filename = ss.str();
		std::ofstream xyz_file(xyz_filename.c_str(),
			std::ofstream::binary);
		xyz_file.write("XYZ1", 4);
		xyz_file.write(reinterpret_cast<char*>(&width), 2);
		xyz_file.write(reinterpret_cast<char*>(&height), 2);
		xyz_file.write(reinterpret_cast<char*>(comp_data), comp_size);
		xyz_file.close();
		delete[] comp_data;
	}

	return 0;
}
