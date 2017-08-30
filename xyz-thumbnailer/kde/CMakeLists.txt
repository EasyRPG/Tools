project(kde-thumbnailer-xyzimage)
cmake_minimum_required(VERSION 3.1)

find_package(ECM 1.0.0 REQUIRED NO_MODULE)

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

find_package(KF5 REQUIRED COMPONENTS KIO)

set(xyzthumbnail_SRCS src/xyzthumbnail.cpp)

add_library(xyzthumbnail MODULE ${xyzthumbnail_SRCS})

target_link_libraries(xyzthumbnail KF5::KIOWidgets ${ZLIB_LIBRARIES})

install(TARGETS xyzthumbnail DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES src/xyzthumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})
