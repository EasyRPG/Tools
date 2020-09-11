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
	std::cerr << "Required options (one of):\n";
	std::cerr << "  -c, --create  Create a new translation\n";
	std::cerr << "  -u, --update  Update an existing translation\n";
	std::cerr << "\n";
	std::cerr << "Optional options:\n";
	std::cerr << "  -h, --help    This usage message\n";
	std::cerr << "  -o, --output  Output directory (default: working directory)\n";
	return 2;
}

std::string encoding;
std::string outdir = ".";
std::vector<std::pair<std::string, std::string>> outdir_files;
bool update = false;

int main(int argc, char** argv) {
	std::string indir;
	bool create = false;

	if (argc <= 1) {
		return print_help(argv);
	}

	/* parse command line arguments */
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];

		if ((arg == "--help") || (arg == "-h")) {
			return print_help(argv);
		} else if ((arg == "--output") || (arg == "-o")) {
			if (i + 1 < argc) {
				outdir = argv[i + 1];
				++i;
			}
		} else if ((arg == "--create") || (arg == "-c")) {
			if (update) {
				return print_help(argv);
			}
			create = true;
		} else if ((arg == "--update") || (arg == "-u")) {
			if (create) {
				return print_help(argv);
			}
			update = true;
		} else {
			indir = arg;
			if (i+1 < argc) {
				encoding = argv[i+1];
			}
			break;
		}
	}

	if (indir.empty() || (!update && !create)) {
		return print_help(argv);
	}

	DIR *dirHandle;
	struct dirent* dirEntry;

	dirHandle = opendir(outdir.c_str());
	if (!dirHandle) {
		std::cerr << "Can not access output directory " << outdir << "\n";
		return 1;
	}
	if (update) {
		while (nullptr != (dirEntry = readdir(dirHandle))) {
			outdir_files.emplace_back(dirEntry->d_name, Utils::LowerCase(dirEntry->d_name));
		}
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

static std::string get_outdir_file(const std::string& file) {
	for (const auto& f : outdir_files) {
		if (f.second == file) {
			return f.first;
		}
	}
	return "";
}

void DumpLdb(const std::string& filename) {
	TranslationLdb t = Translation::fromLDB(filename, encoding);

	auto dump = [](Translation& ti, const std::string& poname) {
		if (update) {
			std::string po = get_outdir_file(Utils::LowerCase(poname + ".po"));
			if (!po.empty()) {
				Translation pot;
				pot = Translation::fromPO(outdir + "/" + po);
				auto stale = ti.Merge(pot);
				if (!stale.getEntries().empty()) {
					std::cout << " " << stale.getEntries().size() << " strings are stale\n";
					std::ofstream outfile(outdir + "/" + poname + ".stale.po");
					stale.write(outfile);
				}
			}
		}

		std::ofstream outfile(outdir + "/" + poname + ".po");
		ti.write(outfile);
	};

	std::cout << " " << t.terms.getEntries().size() << " strings\n";
	dump(t.terms, "RPG_RT.ldb");

	std::cout << " " << t.common_events.getEntries().size() << " strings in Common Events\n";
	dump(t.common_events, "RPG_RT.ldb.common");

	std::cout << " " << t.battle_events.getEntries().size() << " strings in Battle Events\n";
	dump(t.terms, "RPG_RT.ldb.battle");
}

void DumpLmuLmtInner(const std::string& filename, Translation& t, const std::string& poname) {
	Translation pot;

	if (t.getEntries().empty()) {
		std::cout << " Skipped... No strings found.\n";
		return;
	}

	std::cout << " " << t.getEntries().size() << " strings\n";

	if (update) {
		std::string po = get_outdir_file(Utils::LowerCase(poname + ".po"));
		if (!po.empty()) {
			if (poname == "Map0001") {
				int a;
			}

			pot = Translation::fromPO(outdir + "/" + po);
			auto stale = t.Merge(pot);
			if (!stale.getEntries().empty()) {
				std::cout << " " << stale.getEntries().size() << " strings are stale\n";
				std::ofstream outfile(outdir + "/" + poname + ".stale.po");
				stale.write(outfile);
			}
		}
	}

	std::ofstream outfile(outdir + "/" + poname + ".po");

	t.write(outfile);
}

void DumpLmu(const std::string& filename) {
	Translation t = Translation::fromLMU(filename, encoding);
	DumpLmuLmtInner(filename, t, Utils::GetFilename(filename));
}

void DumpLmt(const std::string& filename) {
	Translation t = Translation::fromLMT(filename, encoding);
	DumpLmuLmtInner(filename, t, "RPG_RT.lmt");
}

