project(kde-thumbnailer-xyzimage)
cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 11)

find_package(ECM 1.0.0 REQUIRED NO_MODULE)

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

find_package(KF5 REQUIRED COMPONENTS KIO)

set(xyz_SRCS src/xyz.cpp)

# Thumbnail plugin
add_library(xyzthumbnail MODULE ${xyz_SRCS})
target_link_libraries(xyzthumbnail KF5::KIOWidgets ${ZLIB_LIBRARIES})
install(TARGETS xyzthumbnail DESTINATION ${KDE_INSTALL_QTPLUGINDIR})
install(FILES src/xyzthumbnail.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

# QImageFormats plugin
add_library(kimg_qxyz MODULE ${xyz_SRCS})
target_link_libraries(kimg_qxyz KF5::KIOWidgets ${ZLIB_LIBRARIES})
install(TARGETS kimg_qxyz DESTINATION "${KDE_INSTALL_QTPLUGINDIR}/imageformats")
install(FILES src/xyz.desktop DESTINATION "${KDE_INSTALL_KSERVICES5DIR}/qimageioplugins")
