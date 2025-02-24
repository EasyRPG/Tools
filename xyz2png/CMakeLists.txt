cmake_minimum_required(VERSION 3.16)
project(xyz2png VERSION 1.1 LANGUAGES CXX
	HOMEPAGE_URL "https://easyrpg.org/")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")
include(ConfigureWindows)

find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)

add_executable(xyz2png src/xyz2png.cpp)
target_compile_definitions(xyz2png PRIVATE
	PACKAGE_VERSION="${PROJECT_VERSION}"
	PACKAGE_BUGREPORT="https://github.com/EasyRPG/Tools/issues"
	PACKAGE_URL="${PROJECT_HOMEPAGE_URL}")
target_link_libraries(xyz2png PNG::PNG ZLIB::ZLIB)
target_use_utf8_codepage_on_windows(xyz2png)

include(GNUInstallDirs)
install(TARGETS xyz2png RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
