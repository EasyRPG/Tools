cmake_minimum_required(VERSION 3.7)
project(xyzcrush VERSION 1.0 LANGUAGES C CXX)

find_package(ZLIB REQUIRED)

add_library(zopfli
	src/zopfli/blocksplitter.c
	src/zopfli/cache.c
	src/zopfli/deflate.c
	src/zopfli/gzip_container.c
	src/zopfli/hash.c
	src/zopfli/katajainen.c
	src/zopfli/lz77.c
	src/zopfli/squeeze.c
	src/zopfli/tree.c
	src/zopfli/util.c
	src/zopfli/zlib_container.c
	src/zopfli/zopfli_lib.c)

set_target_properties(zopfli PROPERTIES LINKER_LANGUAGE CXX)

add_executable(xyzcrush src/xyzcrush.cpp)

target_link_libraries(xyzcrush zopfli ZLIB::ZLIB)

include(GNUInstallDirs)
install(TARGETS xyzcrush RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
