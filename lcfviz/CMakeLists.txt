cmake_minimum_required(VERSION 3.7)
project(lcfviz VERSION 1.0 LANGUAGES CXX)

# C++14 is required
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_MODULE_PATH APPEND "${CMAKE_CURRENT_SOURCE_DIR}/../Modules")

include(ConfigureWindows)

find_package(liblcf REQUIRED)

add_executable(lcfviz
	src/dirent_win.h
	src/main.cpp
	src/utils.cpp
	src/utils.h
)

target_link_libraries(lcfviz liblcf::liblcf)

include(GNUInstallDirs)
install(TARGETS lcfviz RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
