cmake_minimum_required(VERSION 3.16)
project(lcfviz VERSION 1.1 LANGUAGES CXX
	HOMEPAGE_URL "https://easyrpg.org/")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")
include(ConfigureWindows)

find_package(liblcf REQUIRED)

set(argparse_dir src/external/argparse)
set(dirent_dir src/external/dirent_win)
add_executable(lcfviz
	src/main.cpp
	src/utils.cpp
	src/utils.h
	${argparse_dir}/argparse.hpp
	${dirent_dir}/dirent_win.h)
target_compile_features(lcfviz PRIVATE cxx_std_17)
target_include_directories(lcfviz PRIVATE
	${argparse_dir} ${dirent_dir})
target_compile_definitions(lcfviz PRIVATE
	PACKAGE_VERSION="${PROJECT_VERSION}"
	PACKAGE_BUGREPORT="https://github.com/EasyRPG/Tools/issues"
	PACKAGE_URL="${PROJECT_HOMEPAGE_URL}")
target_link_libraries(lcfviz liblcf::liblcf)
target_use_utf8_codepage_on_windows(lcfviz)

include(GNUInstallDirs)
install(TARGETS lcfviz RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
