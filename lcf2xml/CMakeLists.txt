cmake_minimum_required(VERSION 3.7)
project(lcf2xml VERSION 1.0 LANGUAGES CXX)

set(CMAKE_MODULE_PATH APPEND "${CMAKE_CURRENT_SOURCE_DIR}/../Modules")

include(ConfigureWindows)

find_package(liblcf REQUIRED)

add_executable(lcf2xml src/main.cpp)

target_link_libraries(lcf2xml liblcf::liblcf)

include(GNUInstallDirs)
install(TARGETS lcf2xml RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
