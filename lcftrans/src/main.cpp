/*
 * Copyright (c) 2020 LcfTrans authors
 * This file is released under the MIT License
 * http://opensource.org/licenses/MIT
 */

#include <cstring>
#include <fstream>
#include <iostream>
#include <lcf/encoder.h>
#include <lcf/reader_util.h>

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
	std::cerr << "\n";
	std::cerr << "When not specified the encoding is read from RPG_RT.ini or auto-detected.\n";
	return 2;
}

std::string encoding;
std::string outdir = ".";
std::vector<std::pair<std::string, std::string>> source_files;
std::vector<std::pair<std::string, std::string>> outdir_files;
std::string ini_file;
std::string database_file;
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

	auto full_path = [&](const auto& name) {
		return indir + "/" + name;
	};

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
			const auto& lname = Utils::LowerCase(dirEntry->d_name);
			source_files.emplace_back(dirEntry->d_name, lname);

			if (lname == INI_FILE) {
				ini_file = full_path(dirEntry->d_name);
			} else if (lname == DATABASE_FILE) {
				database_file = full_path(dirEntry->d_name);
			}
		}
		closedir(dirHandle);
	} else {
		std::cerr << "Failed reading dir " << indir << "\n";
		return 1;
	}

	if (encoding.empty()) {
		if (!ini_file.empty()) {
			encoding = lcf::ReaderUtil::GetEncoding(ini_file);
		}
		if (encoding.empty() && !database_file.empty()) {
			std::ifstream i(database_file, std::ios::binary);
			if (i) {
				encoding = lcf::ReaderUtil::DetectEncoding(i);
			}
		}
	}

	lcf::Encoder enc(encoding);
	if (!enc.IsOk()) {
		std::cerr << "Bad encoding " << encoding << "\n";
		return 3;
	}
	std::cout << "LcfTrans\n";
	std::cout << "Using encoding " << encoding << "\n";

	std::sort(source_files.begin(), source_files.end(), [](const auto& a, const auto& b) {
		return a.first < b.first;
	});

	for (const auto& s : source_files) {
		const auto& name = s.first;
		const auto& lname = s.second;

		if (lname == DATABASE_FILE) {
			std::cout << "Parsing Database " << name << std::endl;
			DumpLdb(full_path(name));
		} else if (lname == MAPTREE_FILE) {
			std::cout << "Parsing Maptree " << name << std::endl;
			DumpLmt(full_path(name));
		} else if (Utils::HasExt(lname, ".lmu")) {
			std::cout << "Parsing Map " << name << std::endl;
			DumpLmu(full_path(name));
		}
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
					std::string term = stale.getEntries().size() == 1 ? " term is " : " terms are ";

					std::cout << " " << stale.getEntries().size() << term << "stale\n";
					std::ofstream outfile(outdir + "/" + poname + ".stale.po");
					stale.write(outfile);
				}
			}
		}

		std::ofstream outfile(outdir + "/" + poname + ".po");
		ti.write(outfile);
	};

	auto term = [](const Translation& t) {
		return std::to_string(t.getEntries().size()) + " " + (t.getEntries().size() == 1 ? "term " : "terms ");
	};

	std::cout << " " << term(t.terms) << "in the database\n";
	dump(t.terms, "RPG_RT.ldb");

	std::cout << " " << term(t.common_events) << "in Common Events\n";
	dump(t.common_events, "RPG_RT.ldb.common");

	std::cout << " " << term(t.battle_events) << "in Battle Events\n";
	dump(t.battle_events, "RPG_RT.ldb.battle");
}

void DumpLmuLmtInner(const std::string& filename, Translation& t, const std::string& poname) {
	Translation pot;

	if (t.getEntries().empty()) {
		std::cout << " Skipped. No terms found.\n";
		return;
	}

	std::cout << " " << t.getEntries().size() << " term" << (t.getEntries().size() == 1 ? "" : "s") << "\n";

	if (update) {
		std::string po = get_outdir_file(Utils::LowerCase(poname + ".po"));
		if (!po.empty()) {
			if (poname == "Map0001") {
				int a;
			}

			pot = Translation::fromPO(outdir + "/" + po);
			auto stale = t.Merge(pot);
			if (!stale.getEntries().empty()) {
				std::string term = stale.getEntries().size() == 1 ? " term is " : " terms are ";
				std::cout << " " << stale.getEntries().size() << term << "stale\n";
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

