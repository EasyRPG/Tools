cmake_minimum_required(VERSION 3.16)
project(lmu2png VERSION 1.2 LANGUAGES CXX
	HOMEPAGE_URL "https://easyrpg.org/")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")
include(ConfigureWindows)

find_package(ZLIB REQUIRED)
find_package(liblcf REQUIRED)
find_package(FreeImage REQUIRED)

set(argparse_dir src/external/argparse)
add_executable(lmu2png
	src/main.h
	src/main.cpp
	src/chipset.h
	src/chipset.cpp
	src/xyzplugin.h
	src/xyzplugin.cpp
	${argparse_dir}/argparse.hpp)
target_compile_features(lmu2png PRIVATE cxx_std_17)
target_include_directories(lmu2png PRIVATE ${argparse_dir})
target_compile_definitions(lmu2png PRIVATE
	PACKAGE_VERSION="${PROJECT_VERSION}"
	PACKAGE_BUGREPORT="https://github.com/EasyRPG/Tools/issues"
	PACKAGE_URL="${PROJECT_HOMEPAGE_URL}")
target_link_libraries(lmu2png ZLIB::ZLIB freeimage::FreeImage liblcf::liblcf)

include(GNUInstallDirs)
install(TARGETS lmu2png RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
