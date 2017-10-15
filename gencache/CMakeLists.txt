cmake_minimum_required(VERSION 3.7)
project(gencache VERSION 1.0 LANGUAGES CXX)

set(CMAKE_MODULE_PATH APPEND "${CMAKE_CURRENT_SOURCE_DIR}/../Modules")

find_package(ICU COMPONENTS uc REQUIRED)

add_executable(gencache src/main.cpp)

target_link_libraries(gencache ICU::uc)

include(GNUInstallDirs)
install(TARGETS gencache RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
