cmake_minimum_required(VERSION 3.7)
project(gencache VERSION 1.1 LANGUAGES CXX)

set(CMAKE_MODULE_PATH APPEND "${CMAKE_CURRENT_SOURCE_DIR}/../Modules")

include(ConfigureWindows)

find_package(ICU COMPONENTS uc data REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(gencache
	src/main.cpp
	src/dirent_win.h
)

target_compile_features(gencache PRIVATE cxx_std_11)
target_compile_definitions(gencache PRIVATE PACKAGE_VERSION="${PROJECT_VERSION}")
target_link_libraries(gencache ICU::uc ICU::data nlohmann_json::nlohmann_json)

include(GNUInstallDirs)
install(TARGETS gencache RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
