cmake_minimum_required(VERSION 3.7)
project(lmu2png VERSION 1.0 LANGUAGES CXX)

set(CMAKE_MODULE_PATH APPEND "${CMAKE_CURRENT_SOURCE_DIR}/../Modules")

find_package(liblcf REQUIRED)
find_package(SDL2_image REQUIRED)

add_executable(lmu2png
	src/chipset.cpp
	src/main.cpp)

target_link_libraries(lmu2png SDL2::IMAGE liblcf::liblcf)

include(GNUInstallDirs)
install(TARGETS lmu2png RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
