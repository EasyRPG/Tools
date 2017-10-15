find_path(LIBLCF_INCLUDE_DIR_INTERNAL liblcf/reader_lcf.h)
find_library(LIBLCF_LIBRARY NAMES lcf liblcf)
if(EXISTS "${LIBLCF_INCLUDE_DIR_INTERNAL}")
	set(LIBLCF_INCLUDE_DIR "${LIBLCF_INCLUDE_DIR_INTERNAL}/liblcf")
endif()

# Manually pull in dependencies because liblcf has no CMake Package config file (yet)
# and otherwise a link against the static library would fail
find_package(ICU COMPONENTS i18n uc data REQUIRED)
find_package(EXPAT)

if(NOT TARGET LIBLCF::LIBLCF)
	add_library(LIBLCF::LIBLCF UNKNOWN IMPORTED)
	set_target_properties(LIBLCF::LIBLCF PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${LIBLCF_INCLUDE_DIR}"
		IMPORTED_LOCATION "${LIBLCF_LIBRARY}"
		INTERFACE_LINK_LIBRARIES ICU::i18n ICU::uc ICU::data)
	if(EXPAT_FOUND)
		set_target_properties(LIBLCF::LIBLCF PROPERTIES
			INTERFACE_LINK_LIBRARIES "${EXPAT_LIBRARY}")
	endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(liblcf DEFAULT_MSG LIBLCF_INCLUDE_DIR LIBLCF_LIBRARY)

set(LIBLCF_INCLUDE_DIRS ${LIBLCF_INCLUDE_DIR})
set(LIBLCF_LIBRARIES ${LIBLCF_LIBRARY})

mark_as_advanced(LIBLCF_INCLUDE_DIR LIBLCF_LIBRARY)
