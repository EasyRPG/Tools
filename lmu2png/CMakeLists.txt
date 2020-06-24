cmake_minimum_required(VERSION 3.7)
project(lmu2png VERSION 1.0 LANGUAGES CXX)

set(CMAKE_MODULE_PATH APPEND "${CMAKE_CURRENT_SOURCE_DIR}/../Modules")

include(ConfigureWindows)

find_package(ZLIB REQUIRED)
find_package(liblcf REQUIRED)
find_package(SDL2_image REQUIRED)

add_executable(lmu2png
	src/sdlxyz.cpp
	src/chipset.cpp
	src/main.cpp)

target_link_libraries(lmu2png ZLIB::ZLIB SDL2::IMAGE liblcf::liblcf)

target_compile_features(lmu2png PRIVATE cxx_std_14)

include(GNUInstallDirs)
install(TARGETS lmu2png RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
