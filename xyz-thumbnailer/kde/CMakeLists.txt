cmake_minimum_required(VERSION 3.16...3.28 FATAL_ERROR)
project(kde-thumbnailer-xyzimage)

set(CMAKE_CXX_STANDARD 17)

option(XYZ_UPDATE_MIMEDB "Whether to run update-mime-database after install (default: ON)" ON)

find_package(ECM 6.0.0 REQUIRED NO_MODULE)

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

find_package(KF6 REQUIRED COMPONENTS KIO)
find_package(Qt6 REQUIRED COMPONENTS Gui)

# Thumbnail plugin
kcoreaddons_add_plugin(xyzthumbnail INSTALL_NAMESPACE "kf6/thumbcreator")
target_sources(xyzthumbnail PRIVATE src/xyz.cpp src/xyz_thumbnail.cpp)
target_link_libraries(xyzthumbnail PRIVATE KF6::KIOWidgets ${ZLIB_LIBRARIES})

# QImageFormats plugin
qt_add_plugin(libqxyz PLUGIN_TYPE imageformats)
target_sources(libqxyz PRIVATE src/xyz.cpp src/xyz_imageio.cpp)
target_link_libraries(libqxyz PRIVATE Qt6::Gui ${ZLIB_LIBRARIES})
set_target_properties(libqxyz PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/imageformats)
install(TARGETS libqxyz DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/imageformats)

# mime types
install(
	FILES ${CMAKE_CURRENT_SOURCE_DIR}/integration/image-xyz.xml
	DESTINATION ${CMAKE_INSTALL_DATADIR}/mime/packages
)

if(XYZ_UPDATE_MIMEDB AND NOT CMAKE_CROSSCOMPILING)
	find_program(UPDATE_MIME_DATABASE update-mime-database)
endif()

if(XYZ_UPDATE_MIMEDB AND UPDATE_MIME_DATABASE)
	install(CODE
		"execute_process(COMMAND ${UPDATE_MIME_DATABASE}
		\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/mime)"
	)
endif()
