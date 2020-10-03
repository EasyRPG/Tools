cmake_minimum_required(VERSION 3.7)
project(xyz_thumbnailer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_MODULE_PATH APPEND "${CMAKE_CURRENT_SOURCE_DIR}/../../Modules")

include(ConfigureWindows)

find_package(ZLIB REQUIRED)

add_library(xyz_thumbnailer SHARED
	ClassFactory.cpp
	dllmain.cpp
	Reg.cpp
	RpgMakerXyzThumbnailProvider.cpp
)

set_target_properties(xyz_thumbnailer PROPERTIES OUTPUT_NAME "EasyRpgXyzShellExtThumbnailHandler")

target_link_libraries(xyz_thumbnailer ZLIB::ZLIB)

include(GNUInstallDirs)
install(TARGETS xyz_thumbnailer RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/xyz_thumbnailer/${MSVC_CXX_ARCHITECTURE_ID})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/ DESTINATION ${CMAKE_INSTALL_BINDIR}/xyz_thumbnailer)
