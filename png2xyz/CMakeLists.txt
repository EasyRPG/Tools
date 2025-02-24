cmake_minimum_required(VERSION 3.16)
project(png2xyz VERSION 1.1 LANGUAGES CXX
	HOMEPAGE_URL "https://easyrpg.org/")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")
include(ConfigureWindows)

find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)

add_executable(png2xyz src/png2xyz.cpp)
target_compile_definitions(png2xyz PRIVATE
	PACKAGE_VERSION="${PROJECT_VERSION}"
	PACKAGE_BUGREPORT="https://github.com/EasyRPG/Tools/issues"
	PACKAGE_URL="${PROJECT_HOMEPAGE_URL}")
target_link_libraries(png2xyz PNG::PNG ZLIB::ZLIB)
target_use_utf8_codepage_on_windows(png2xyz)

include(GNUInstallDirs)
install(TARGETS png2xyz RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
