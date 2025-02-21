cmake_minimum_required(VERSION 3.16)
project(lmu2png VERSION 2.0 LANGUAGES CXX
	HOMEPAGE_URL "https://easyrpg.org/")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")
include(ConfigureWindows)

find_package(ZLIB REQUIRED)
find_package(liblcf REQUIRED)
find_package(FreeImage REQUIRED)

set(WITH_GUI "Automatic" CACHE STRING "Build a GUI frontend (ON/OFF/Automatic), Default: Automatic")
set_property(CACHE WITH_GUI PROPERTY STRINGS ON OFF Automatic)
if(WITH_GUI STREQUAL "Automatic")
	find_package(wxWidgets CONFIG)
elseif(WITH_GUI STREQUAL "ON")
	find_package(wxWidgets CONFIG REQUIRED)
endif()

set(argparse_dir src/external/argparse)
add_executable(lmu2png
	src/main.h
	src/main.cpp
	src/chipset.h
	src/chipset.cpp
	src/xyzplugin.h
	src/xyzplugin.cpp
	src/utils.h
	src/utils.cpp
	${argparse_dir}/argparse.hpp)
target_compile_features(lmu2png PRIVATE cxx_std_17)
target_include_directories(lmu2png PRIVATE ${argparse_dir})
target_compile_definitions(lmu2png PRIVATE
	PACKAGE_VERSION="${PROJECT_VERSION}"
	PACKAGE_BUGREPORT="https://github.com/EasyRPG/Tools/issues"
	PACKAGE_URL="${PROJECT_HOMEPAGE_URL}")
target_link_libraries(lmu2png ZLIB::ZLIB freeimage::FreeImage liblcf::liblcf)
target_use_utf8_codepage_on_windows(lmu2png)

if(wxWidgets_FOUND)
	target_compile_definitions(lmu2png PRIVATE WITH_GUI)
	target_sources(lmu2png PRIVATE
		src/gui.h
		src/gui.cpp)
	target_link_libraries(lmu2png wx::base wx::core)
	set(GUI_STATUS "Enabled (wxWidgets ${wxWidgets_VERSION})")
else()
	set(GUI_STATUS "Disabled")
endif()
message(STATUS "GUI is ${GUI_STATUS}")

include(GNUInstallDirs)
install(TARGETS lmu2png RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
