cmake_minimum_required(VERSION 3.7)
project(xyz2png VERSION 1.0 LANGUAGES CXX)

find_package(PNG REQUIRED)

add_executable(xyz2png src/xyz2png.cpp)

target_link_libraries(xyz2png PNG::PNG)

include(GNUInstallDirs)
install(TARGETS xyz2png RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
