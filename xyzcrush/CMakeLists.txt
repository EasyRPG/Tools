cmake_minimum_required(VERSION 3.16)
project(xyzcrush VERSION 1.1 LANGUAGES C CXX
	HOMEPAGE_URL "https://easyrpg.org/")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")
include(ConfigureWindows)

find_package(ZLIB REQUIRED)

set(zopfli_dir src/external/zopfli)
add_library(zopfli STATIC
	${zopfli_dir}/zopfli.h
	${zopfli_dir}/blocksplitter.h
	${zopfli_dir}/blocksplitter.c
	${zopfli_dir}/cache.h
	${zopfli_dir}/cache.c
	${zopfli_dir}/deflate.h
	${zopfli_dir}/deflate.c
	${zopfli_dir}/hash.h
	${zopfli_dir}/hash.c
	${zopfli_dir}/katajainen.h
	${zopfli_dir}/katajainen.c
	${zopfli_dir}/lz77.h
	${zopfli_dir}/lz77.c
	${zopfli_dir}/squeeze.h
	${zopfli_dir}/squeeze.c
	${zopfli_dir}/symbols.h
	${zopfli_dir}/tree.h
	${zopfli_dir}/tree.c
	${zopfli_dir}/util.h
	${zopfli_dir}/util.c
	${zopfli_dir}/zlib_container.h
	${zopfli_dir}/zlib_container.c)
target_include_directories(zopfli INTERFACE ${zopfli_dir})
set_target_properties(zopfli PROPERTIES LINKER_LANGUAGE CXX)

add_executable(xyzcrush src/xyzcrush.cpp)
target_compile_definitions(xyzcrush PRIVATE
	PACKAGE_VERSION="${PROJECT_VERSION}"
	PACKAGE_BUGREPORT="https://github.com/EasyRPG/Tools/issues"
	PACKAGE_URL="${PROJECT_HOMEPAGE_URL}")
target_link_libraries(xyzcrush zopfli ZLIB::ZLIB)
target_use_utf8_codepage_on_windows(xyzcrush)

include(GNUInstallDirs)
install(TARGETS xyzcrush RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
