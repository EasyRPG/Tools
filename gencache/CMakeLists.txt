cmake_minimum_required(VERSION 3.16)
project(gencache VERSION 1.2 LANGUAGES CXX
	HOMEPAGE_URL "https://easyrpg.org/")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")
include(ConfigureWindows)

find_package(ICU COMPONENTS uc data REQUIRED)
find_package(nlohmann_json REQUIRED)

set(dirent_dir src/external/dirent_win)
add_executable(gencache
	src/main.cpp
	${dirent_dir}/dirent_win.h)
target_compile_features(gencache PRIVATE cxx_std_17)
target_include_directories(gencache PRIVATE ${dirent_dir})
target_compile_definitions(gencache PRIVATE
	PACKAGE_VERSION="${PROJECT_VERSION}"
	PACKAGE_BUGREPORT="https://github.com/EasyRPG/Tools/issues"
	PACKAGE_URL="${PROJECT_HOMEPAGE_URL}")
target_link_libraries(gencache ICU::uc ICU::data nlohmann_json::nlohmann_json)
target_use_utf8_codepage_on_windows(gencache)

include(GNUInstallDirs)
install(TARGETS gencache RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
