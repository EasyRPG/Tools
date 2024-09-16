# Find package module for FreeImage library.
#
# The following variables are set by this module:
#
#   FreeImage_FOUND: TRUE if FreeImage is found.
#   freeimage::FreeImage: Imported target to link against.
#
# The following variables control the behavior of this module:
#
# FREEIMAGE_INCLUDE_DIR_HINTS: List of additional directories in which to
#                              search for FreeImage includes.
# FREEIMAGE_LIBRARY_DIR_HINTS: List of additional directories in which to
#                              search for FreeImage libraries.

set(FREEIMAGE_INCLUDE_DIR_HINTS "" CACHE PATH "FreeImage include directory")
set(FREEIMAGE_LIBRARY_DIR_HINTS "" CACHE PATH "FreeImage library directory")

include(FindPackageHandleStandardArgs)

find_package(FreeImage CONFIG QUIET)
if(TARGET freeimage::FreeImage)
	message(STATUS "Found FreeImage ${FreeImage_VERSION}")
endif()

if(NOT FreeImage_FOUND)
	find_path(FREEIMAGE_INCLUDE_DIRS
		NAMES
			FreeImage.h
		HINTS
			${FREEIMAGE_INCLUDE_DIR_HINTS}
		PATHS
			${FREEIMAGE_ROOT}
	)
	find_library(FREEIMAGE_LIBRARIES
		NAMES
			freeimage
		HINTS
			${FREEIMAGE_LIBRARY_DIR_HINTS}
		PATHS
			${FREEIMAGE_ROOT}
	)

	if(FREEIMAGE_INCLUDE_DIRS AND EXISTS "${FREEIMAGE_INCLUDE_DIRS}/FreeImage.h")
		file(STRINGS "${FREEIMAGE_INCLUDE_DIRS}/FreeImage.h" _VERSION_MAJOR_LINE REGEX "^#define[ \t]+FREEIMAGE_MAJOR_VERSION[ \t]+[0-9]+$")
		file(STRINGS "${FREEIMAGE_INCLUDE_DIRS}/FreeImage.h" _VERSION_MINOR_LINE REGEX "^#define[ \t]+FREEIMAGE_MINOR_VERSION[ \t]+[0-9]+$")
		file(STRINGS "${FREEIMAGE_INCLUDE_DIRS}/FreeImage.h" _VERSION_PATCH_LINE REGEX "^#define[ \t]+FREEIMAGE_RELEASE_SERIAL[ \t]+[0-9]+$")
		string(REGEX REPLACE "^#define[ \t]+FREEIMAGE_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" _VERSION_MAJOR "${_VERSION_MAJOR_LINE}")
		string(REGEX REPLACE "^#define[ \t]+FREEIMAGE_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" _VERSION_MINOR "${_VERSION_MINOR_LINE}")
		string(REGEX REPLACE "^#define[ \t]+FREEIMAGE_RELEASE_SERIAL[ \t]+([0-9]+)$" "\\1" _VERSION_PATCH "${_VERSION_PATCH_LINE}")
		set(FREEIMAGE_VERSION_STRING ${_VERSION_MAJOR}.${_VERSION_MINOR}.${_VERSION_PATCH})
		unset(_VERSION_MAJOR_LINE)
		unset(_VERSION_MINOR_LINE)
		unset(_VERSION_PATCH_LINE)
		unset(_VERSION_MAJOR)
		unset(_VERSION_MINOR)
		unset(_VERSION_PATCH)
	endif()

	find_package_handle_standard_args(FreeImage
		REQUIRED_VARS FREEIMAGE_INCLUDE_DIRS FREEIMAGE_LIBRARIES
		VERSION_VAR FREEIMAGE_VERSION_STRING)

	add_library(freeimage::FreeImage INTERFACE IMPORTED)
	target_include_directories(
		freeimage::FreeImage INTERFACE ${FREEIMAGE_INCLUDE_DIRS})
	target_link_libraries(
		freeimage::FreeImage INTERFACE ${FREEIMAGE_LIBRARIES})

	mark_as_advanced(FREEIMAGE_INCLUDE_DIRS FREEIMAGE_LIBRARIES)
endif()
