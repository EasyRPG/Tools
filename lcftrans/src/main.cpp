/*
 * Copyright (c) 2020 LcfTrans authors
 * This file is released under the MIT License
 * http://opensource.org/licenses/MIT
 */

#include <cstring>
#include <fstream>
#include <iostream>

#include "translation.h"
#include "utils.h"

#ifdef _WIN32
#include "dirent_win.h"
#else
#include <dirent.h>
#endif

#define DATABASE_FILE "rpg_rt.ldb"
#define MAPTREE_FILE "rpg_rt.lmt"
#define INI_FILE "rpg_rt.ini"

void DumpLdb(const std::string& filename);
void DumpLmu(const std::string& filename);
void DumpLmt(const std::string& filename);

static int print_help(char** argv) {
	std::cerr << "lcftrans - Translate RPG Maker 2000/2003 projects\n";
	std::cerr << "Usage: " << argv[0] << " [OPTION...] DIRECTORY [ENCODING]\n";
	std::cerr << "Options:\n";
	std::cerr << "  -h, --help    This usage message\n";
	std::cerr << "  -o, --output  Output directory (default: working directory)\n";
	return 2;
}

std::string encoding;
std::string outdir = ".";

int main(int argc, char** argv) {
	std::string indir;

	if (argc <= 1) {
		return print_help(argv);
	}

	/* parse command line arguments */
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];

		if ((arg == "--help") || (arg == "-h")) {
			return print_help(argv);
		} else if ((arg == "--output") || (arg == "-o")) {
			if (i+1 < argc) {
				outdir = argv[i+1];
				++i;
			}
		} else {
			indir = arg;
			if (i+1 < argc) {
				encoding = argv[i+1];
			}
			break;
		}
	}

	if (indir.empty()) {
		return print_help(argv);
	}

	DIR *dirHandle;
	struct dirent* dirEntry;

	dirHandle = opendir(outdir.c_str());
	if (!dirHandle) {
		std::cerr << "Can not access output directory " << outdir << "\n";
		return 1;
	}
	closedir(dirHandle);

	dirHandle = opendir(indir.c_str());

	if (dirHandle) {
		while (nullptr != (dirEntry = readdir(dirHandle))) {
			std::string name = dirEntry->d_name;
			std::string lname = Utils::LowerCase(name);

			auto full_path = [&]() {
				return indir + "/" + name;
			};

			if (lname == DATABASE_FILE) {
				std::cout << "Parsing Database " << name << std::endl;
				DumpLdb(full_path());
			} else if (lname == MAPTREE_FILE) {
				std::cout << "Parsing Maptree " << name << std::endl;
				DumpLmt(full_path());
			} else if (Utils::HasExt(lname, ".lmu")) {
				std::cout << "Parsing Map " << name << std::endl;
				DumpLmu(full_path());
			}
		}
		closedir(dirHandle);
	} else {
		std::cerr << "Failed reading dir " << indir << "\n";
		return 1;
	}

	return 0;
}

void DumpLdb(const std::string& filename) {
	TranslationLdb t = Translation::fromLDB(filename, encoding);

	std::cout << " " << t.terms.getEntries().size() << " strings\n";
	std::cout << " " << t.common_events.getEntries().size() << " strings in Common Events\n";
	std::cout << " " << t.battle_events.getEntries().size() << " strings in Battle Events\n";

	{
		std::ofstream outfile(outdir + "/RPG_RT.ldb.po");
		t.terms.write(outfile);
	}
	{
		std::ofstream outfile(outdir + "/RPG_RT.ldb.common.po");
		t.common_events.write(outfile);
	}
	{
		std::ofstream outfile(outdir + "/RPG_RT.ldb.battle.po");
		t.battle_events.write(outfile);
	}
}

void DumpLmu(const std::string& filename) {
	Translation t = Translation::fromLMU(filename, encoding);

	if (t.getEntries().empty()) {
		std::cout << " Skipped... No strings found.\n";
		return;
	}

	std::cout << " " << t.getEntries().size() << " strings\n";

	std::ofstream outfile(outdir + "/" + Utils::GetFilename(filename) + ".po");

	t.write(outfile);
}

void DumpLmt(const std::string& filename) {
	Translation t = Translation::fromLMT(filename, encoding);

	if (t.getEntries().empty()) {
		std::cout << " Skipped... No strings found.\n";
		return;
	}

	std::cout << " " << t.getEntries().size() << " strings\n";

	std::ofstream outfile(outdir + "/RPG_RT.lmt.po");

	t.write(outfile);
}

