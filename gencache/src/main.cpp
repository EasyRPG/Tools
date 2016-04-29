/*
 * Copyright (c) 2016 gencache authors
 * This file is released under the MIT License
 * http://opensource.org/licenses/MIT
 */

#include <unicode/unistr.h>
#include <unicode/ustream.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>

#include "json.hpp"

/* legacy mode, linear list */
#define LEGACY 1

using json = nlohmann::json;

json parse_dir_recursive(const std::string& path, const int depth) {
	DIR *dir;
	struct dirent *dent;
	json r;

	/* do not recurse any further */
	if(depth == 0)
		return r;

	dir = opendir(path.c_str());
	if(dir != nullptr) {
		while((dent = readdir(dir)) != nullptr) {
			std::string dirname = std::string(dent->d_name);
			/* unicode aware lowercase conversion */
			std::string lower_dirname;
			icu::UnicodeString(dent->d_name).toLower().toUTF8String(lower_dirname);

			/* dig deeper, but skip upper and current directory */
			if(dent->d_type == DT_DIR && dirname != ".." && dirname != ".") {
				json temp = parse_dir_recursive(path + "/" + dirname, depth - 1);
				if (!temp.empty()) {
#if LEGACY
					for (json::iterator it = temp.begin(); it != temp.end(); ++it) {
						std::string val = it.value();
						std::string key = it.key();
						r[lower_dirname + "/" + key.substr(0, key.find_last_of("."))] = dirname + "/" + val;
					}
#else
					r[lower_dirname] = temp;
#endif
				}
			}

			/* add files */
			if(dent->d_type == DT_REG || dent->d_type == DT_LNK)
				r[lower_dirname] = dirname;
		}
	}
	closedir(dir);

	return r;
}

int main(int argc, const char* argv[]) {
	struct stat path_info;

	/* defaults */
	int recursion_depth = 3;
	bool pretty_print = false;
	std::string path = ".";
	std::string output = "index.json";

	/* parse command line arguments */
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];

		if ((arg == "--help") || (arg == "-h")) {
			std::cout << "gencache - JSON cache generator for EasyRPG Player ports" << std::endl << std::endl;
			std::cout << "Usage: gencache [ Options ] [ Directory ]" << std::endl;
			std::cout << "Options:" << std::endl;
			std::cout << "  -h, --help             This usage message" << std::endl;
			std::cout << "  -p, --pretty           Pretty print the JSON contents" << std::endl;
			std::cout << "  -o, --output <file>    Output file name (default: \"" << output << "\")" << std::endl;
			std::cout << "  -r, --recurse <depth>  Recursion depth (default: " << std::to_string(recursion_depth) << ")" << std::endl << std::endl;
			std::cout << "It uses the current directory if not given as argument." << std::endl;
			return 0;
		} else if((arg == "--pretty") || (arg == "-p")) {
			pretty_print = true;
		} else if((arg == "--output") || (arg == "-o")) {
			if (i + 1 < argc) {
				output = argv[++i];
			} else {
				std::cerr << "--output without file name argument" << std::endl;
				return 1;
			}
		} else if((arg == "--recurse") || (arg == "-r")) {
			if (i + 1 < argc) {
				std::istringstream iss(argv[++i]);
				if (!(iss >> recursion_depth)) {
					std::cerr << "--recurse option needs a number argument" << std::endl;
					return 1;
				}
			} else {
				std::cerr << "--recurse without depth argument" << std::endl;
				return 1;
			}
		} else {
			if (path == ".") {
				if(stat(arg.c_str(), &path_info) != 0) {
					std::cerr << "Cannot access directory: \"" << arg << "\"" << std::endl;
					return 1;
				} else if(!(path_info.st_mode & S_IFDIR)) {
					std::cerr << "Not a directory: \"" << arg << "\"" << std::endl;
					return 1;
				} else {
					path = arg;
				}
			} else {
				std::cerr << "Skipping additional argument: \"" << arg << "\"..." << std::endl;
			}
		}
	}

	/* get directory contents */
	json cache = parse_dir_recursive(path, recursion_depth);

	/* write to cache file */
	std::ofstream cache_file;
	cache_file.open(output);
	cache_file << cache.dump(pretty_print ? 2 : -1);
	cache_file.close();
	std::cout << "JSON cache has been written to \"" << output << "\"." << std::endl;

	return 0;
}
