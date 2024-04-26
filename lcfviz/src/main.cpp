/*
 * Copyright (c) 2020 LcfViz authors
 * This file is released under the MIT License
 * http://opensource.org/licenses/MIT
 */

// Important:
// Do not write diagnostics to stdout (dot file output uses it), always use stderr

#include <algorithm>
#include <climits>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <argparse.hpp>
#include <lcf/encoder.h>
#include <lcf/reader_util.h>
#include <lcf/lmt/reader.h>
#include <lcf/lmu/reader.h>
#include <lcf/rpg/treemap.h>
#include <lcf/ldb/reader.h>

#include "utils.h"

#ifdef _WIN32
#  include <dirent_win.h>
#else
#  include <dirent.h>
#endif

#define DATABASE_FILE "rpg_rt.ldb"
#define MAPTREE_FILE "rpg_rt.lmt"
#define INI_FILE "rpg_rt.ini"

void ParseLmt(const std::string& filename);

const std::string help_epilog = R"(Example usage:
  lcfviz YOURGAME | dot -Goverlap=false -Gsplines=true -Tpng -o graph.png
Creates an overlap-free, directed graph (for huge graphs use sfdp, not dot)

)" "Homepage " PACKAGE_URL " - Report bugs at: " PACKAGE_BUGREPORT;

static int print_help(char** argv) {
	std::cerr << "lcfviz - Creates a dot file from a map tree by scanning the map for teleports.\n";
	std::cerr << "Usage: " << argv[0] << " [OPTION...] DIRECTORY [ENCODING]\n";
	std::cerr << "Options:\n";
	std::cerr << "  -d, --depth DEPTH  Maximal depth from the start node (default: no limit)\n";
	std::cerr << "                     Enables unreachable node detection (-r)\n";
	std::cerr << "  -h, --help         This usage message\n";
	std::cerr << "  -o, --output FILE  Output file (default: stdout)\n";
	std::cerr << "  -r, --remove       Remove nodes that are unreachable from the start node\n";
	std::cerr << "  -s, --start ID     Initial node of the graph (default: start party position)\n";
	std::cerr << "\n";
	std::cerr << "When not specified the encoding is read from RPG_RT.ini or auto-detected.\n";
	std::cerr << "\n";
	std::cerr << "Example usage:\n";
	std::cerr << " lcfviz YOURGAME | dot -Goverlap=false -Gsplines=true -Tpng -o graph.png\n";
	std::cerr << "Creates an overlap-free, directed graph (for huge graphs use sfdp, not dot)\n";
	return 2;
}

namespace {
	/* config */
	std::string encoding, indir;
	std::vector<std::pair<std::string, std::string>> source_files;
	std::vector<std::pair<int, int>> targets_per_map;
	std::vector<std::pair<int, std::string>> maps;

	int depth_limit = -1;
	std::string outfile;
	bool remove_unreachable = false;
	int start_map_id = -1;
}

int main(int argc, char** argv) {
	/*add usage and help messages */
	argparse::ArgumentParser cli("lcfviz", PACKAGE_VERSION);
	cli.set_usage_max_line_width(100);
	cli.add_description("Creates a dot file from a map tree by scanning the map for teleports.");
	cli.add_epilog(help_epilog);

	/* parse command line arguments */
	cli.add_argument("DIRECTORY").default_value(".").store_into(indir)
		.help("Game directory").metavar("DIRECTORY");
	cli.add_argument("-d", "--depth").store_into(depth_limit).scan<'i', int>()
		.help("Maximal depth from the start node (default: no limit).\n"
				"Enables unreachable node detection (-r)").metavar("DEPTH");
	cli.add_argument("-o", "--output").store_into(outfile).metavar("FILE")
		.help("Output file (default: stdout)");
	cli.add_argument("-e", "--encoding").store_into(encoding).metavar("ENC")
		.help("When not specified, is read from RPG_RT.ini or auto-detected");
	cli.add_argument("-r", "--remove").store_into(remove_unreachable)
		.help("Remove nodes that are unreachable from the start node");
	cli.add_argument("-s", "--start").store_into(start_map_id).scan<'i', int>()
		.help("Initial node of the graph (default: start party position)")
		.metavar("ID");
	// for old encoding argument
	cli.add_argument("additional").remaining().hidden();

	try {
		cli.parse_args(argc, argv);
	} catch (const std::exception& err) {
		std::cerr << err.what() << "\n";
		// print usage message
		std::cerr << cli.usage() << "\n";
		std::exit(EXIT_FAILURE);
	}

	// depth arg also sets remove option
	if(cli.is_used("--depth")) {
		remove_unreachable = true;
	}

	// for old encoding argument
	if (auto additional_args = cli.present<std::vector<std::string>>("additional")) {
		if(additional_args.value().size() > 1) {
			std::cerr << "Found additional, unrecognized arguments.\n";
			std::cerr << cli.usage() << "\n";
			std::exit(EXIT_FAILURE);
		} else {
			std::cerr << "Specifying ENCODING as last argument is deprecated, `-e ENC` is the replacement.\n";
			encoding = additional_args.value()[0];
		}
	}

	auto full_path = [&](const auto& name) {
		return indir + "/" + name;
	};

	std::ofstream ofile;
	std::ostream* out = &std::cout;
	if (!outfile.empty()) {
		ofile.open(outfile);
		if (!ofile) {
			std::cerr << "Failed opening outfile " << outfile << "\n";
			return 1;
		}
		out = &ofile;
	}

	DIR *dirHandle;
	struct dirent* dirEntry;

	std::string ini_file;
	std::string database_file;
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
			auto db = lcf::LDB_Reader::Load(database_file, encoding);
			if (db) {
				encoding = lcf::ReaderUtil::DetectEncoding(*db);
			}
		}
	}

	lcf::Encoder enc(encoding);
	if (!enc.IsOk()) {
		std::cerr << "Bad encoding " << encoding << "\n";
		return 3;
	}
	std::cerr << "Using encoding " << encoding << "\n";

	std::sort(source_files.begin(), source_files.end(), [](const auto& a, const auto& b) {
		return a.first < b.first;
	});

	bool parsed_lmt = false;
	// Only process maps that are part of the map tree
	for (const auto& s : source_files) {
		const auto& name = s.first;
		const auto& lname = s.second;

		if (lname == MAPTREE_FILE) {
			std::cerr << "Parsing Maptree " << name << "\n";
			ParseLmt(full_path(name));
			parsed_lmt = true;
		}
	}

	if (!parsed_lmt) {
		std::cerr << "Map Tree (RPG_RT.lmt) not found in " << indir << "\n";
		return 1;
	}

	std::stable_sort(targets_per_map.begin(), targets_per_map.end(), [](const auto& a, const auto& b) {
		return a.first < b.first;
	});
	std::stable_sort(maps.begin(), maps.end(), [](const auto& a, const auto& b) {
		return std::tie(a.first, a.second) < std::tie(b.first, b.second);
	});
	targets_per_map.erase(std::unique(targets_per_map.begin(), targets_per_map.end()), targets_per_map.end());

	// Detect nodes that are unreachable from the start map
	// Pair of: map ID, highest depth seen
	std::vector<std::pair<int,int>> visited;
	depth_limit = depth_limit < 0 ? INT_MAX : depth_limit;
	std::function<void(int,int)> depth_search;
	depth_search = [&](int cur_node, int cur_depth) {
		auto v_it = std::find_if(visited.begin(), visited.end(), [&](const auto &val) {
			return val.first == cur_node;
		});
		if (cur_depth > depth_limit) {
			return;
		}
		if (v_it == visited.end()) {
			visited.emplace_back(cur_node, cur_depth);
		} else {
			if (v_it->second < cur_depth) {
				// node was already seen on a lower level -> abort
				return;
			} else {
				v_it->second = cur_depth;
			}
		}

		auto it = std::find_if(targets_per_map.begin(), targets_per_map.end(), [&](const auto& val) {
			return val.first == cur_node;
		});
		for(; it->first == cur_node; ++it) {
			depth_search(it->second, cur_depth + 1);
		}
	};
	if (remove_unreachable) {
		depth_search(start_map_id, 0);
	}

	*out << "strict digraph G {\n";

	// Output all nodes (if reachable)
	for (const auto& map : maps) {
		if (remove_unreachable && std::find_if(visited.begin(), visited.end(), [&](const auto& val) {
			return val.first == map.first;
		}) == visited.end()){
			continue;
		}

		*out << map.first << " [label=\"" << map.second << "\"";
		if (start_map_id == map.first) {
			*out << " shape=box style=filled fillcolor=gray";
		}
		*out << "];\n";
	}

	// Output edges
	std::vector<std::pair<int, int>> pending_erase;
	for (const auto& map : targets_per_map) {
		if (remove_unreachable && (
			std::find_if(visited.begin(), visited.end(), [&](const auto& val) {
				return val.first == map.first;
			}) == visited.end() ||
			std::find_if(visited.begin(), visited.end(), [&](const auto& val) {
				return val.first == map.second;
			}) == visited.end())
			) {
			// Target or source node not in set
			continue;
		}

		if (std::find_if(pending_erase.begin(), pending_erase.end(), [&](const auto& val) {
			return map.first == val.first && map.second == val.second;
		}) != pending_erase.end()) {
			// Is reverse edge of a bidirectional node
			continue;
		};

		// Detect bidirection
		bool both = false;
		auto find_it = std::find_if(targets_per_map.begin(), targets_per_map.end(), [&](const auto& val) {
			return val.second == map.first && val.first == map.second;
		});
		if (find_it != targets_per_map.end()) {
			both = true;
			pending_erase.emplace_back(find_it->first, find_it->second);
		}

		*out << map.first << " -> " << map.second;
		if (both) {
			*out << " [dir=both]";
		}
		*out << ";\n";
	}
	*out << "}\n";

	return 0;
}

template<typename T>
void parse_event(const lcf::ContextStructBase<T>&, int) {}

template<>
void parse_event(const lcf::ContextStructBase<lcf::rpg::EventCommand>& ctx, int id) {
	if (ctx.name != "string") {
		return;
	}

	if (static_cast<lcf::rpg::EventCommand::Code>(ctx.obj->code) == lcf::rpg::EventCommand::Code::Teleport) {
		int target_id = ctx.obj->parameters[0];
		if (id != target_id)
			targets_per_map.emplace_back(id, target_id);
	}
}

void ParseLmu(const std::string& filename, int id) {
	const auto map = lcf::LMU_Reader::Load(filename, encoding);
	if (!map) {
		return;
	}

	lcf::rpg::ForEachString(*map.get(), [&](const auto&, const auto& ctx) {
		parse_event(ctx, id);
	});
}

template<typename T>
void parse_map(const lcf::ContextStructBase<T>&) {}

template<>
void parse_map(const lcf::ContextStructBase<lcf::rpg::MapInfo>& ctx) {
	if (ctx.obj->type != lcf::rpg::TreeMap::MapType_map) {
		return;
	}

	maps.emplace_back(ctx.obj->ID, lcf::ToString(ctx.obj->name));
	int id = ctx.obj->ID;
	std::string name = "map";
	if (id < 10) {
		name += "000";
	} else if (id < 100) {
		name += "00";
	} else if (id < 1000) {
		name += "0";
	}
	name += std::to_string(id) + ".lmu";
	auto res = std::find_if(source_files.begin(), source_files.end(), [&](const auto& val) {
		return val.second == name;
	});

	auto full_path = [&](const auto& name) {
		return indir + "/" + name;
	};

	if (res != source_files.end()) {
		std::cerr << "Parsing Map " << res->first << "\n";
		ParseLmu(full_path(res->first), id);
	}
}

void ParseLmt(const std::string& filename) {
	auto tree = lcf::LMT_Reader::Load(filename, encoding);

	if (!tree) {
		std::cerr << "Error loading maptree " << filename << "\n";
		return;
	}

	if (start_map_id == -1) {
		start_map_id = tree->start.party_map_id;
	}

	lcf::rpg::ForEachString(*tree, [&](const auto& val, const auto& ctx) {
		if (ctx.name == "name") {
			parse_map(ctx);
		}
	});
}
