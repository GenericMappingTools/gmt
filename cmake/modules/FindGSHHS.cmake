#
# $Id$
#
# Locate GSHHS shorelines
#
# This module accepts the following environment variables:
#
#    GSHHS_ROOT         - Specify the location of GSHHS
#
# This module defines the following CMake variables:
#
#    GSHHS_FOUND        - True if GSHHS is found
#    GSHHS_PATH         - A variable pointing to the GSHHS path
#    GSHHS_VERSION      - String of the GSHHS version found
#    GSHHS_MIN_REQUIRED - Struct of {major, minor, patch} version required
#    GSHHS_EXT          - GSHHS netCDF file extension

# get GSHHS path
find_path (GSHHS_PATH
	NAMES binned_GSHHS_c.nc binned_GSHHS_c.cdf
	HINTS ${GSHHS_ROOT}
	PATH_SUFFIXES
	gshhs+wdbii-gmt
	gshhs+wdbii
	coast
	share/gshhs+wdbii-gmt
	share/gshhs+wdbii
	share/coast
	share/gmt/coast
	PATHS
	${CMAKE_SOURCE_DIR}
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt)

# get GSHHS file
if (GSHHS_PATH)
	find_file (_GSHHS_FILE
		NAMES binned_GSHHS_c.cdf binned_GSHHS_c.nc
		HINTS ${GSHHS_PATH})
endif (GSHHS_PATH)

# check GSHHS version
set (MIN_REQUIRED_GSHHS_VERSION "${MIN_REQUIRED_GSHHS_VERSION_MAJOR}.${MIN_REQUIRED_GSHHS_VERSION_MINOR}.${MIN_REQUIRED_GSHHS_VERSION_PATCH}")
if (_GSHHS_FILE AND NOT GSHHS_FOUND)
	try_run (_EXIT_GSHHS_VERSION _COMPILED_GSHHS_VERSION
		${CMAKE_BINARY_DIR}/CMakeTmp
		${CMAKE_CURRENT_SOURCE_DIR}/gshhs_version.c
		CMAKE_FLAGS
		-DINCLUDE_DIRECTORIES=${NETCDF_INCLUDE_DIR}
		-DLINK_LIBRARIES=${NETCDF_LIBRARIES}
		COMPILE_DEFINITIONS -DSTANDALONE
		RUN_OUTPUT_VARIABLE _GSHHS_VERSION_STRING
		ARGS ${_GSHHS_FILE} ${MIN_REQUIRED_GSHHS_VERSION})

	# check version string
	if (_COMPILED_GSHHS_VERSION)
		# strip whitespace
		string (STRIP ${_GSHHS_VERSION_STRING} GSHHS_VERSION)
		if (_EXIT_GSHHS_VERSION EQUAL 0)
			# found GSHHS of required version or higher
			set (GSHHS_VERSION ${GSHHS_VERSION} CACHE INTERNAL "GSHHS version")
			get_filename_component (GSHHS_EXT ${_GSHHS_FILE} EXT CACHE)
		elseif (_EXIT_GSHHS_VERSION EQUAL -1)
			# found GSHHS but version is too old
			message (WARNING "GSHHS found but it is too old (${GSHHS_VERSION}). "
				"Need at least ${MIN_REQUIRED_GSHHS_VERSION}.")
		endif (_EXIT_GSHHS_VERSION EQUAL 0)
	endif (_COMPILED_GSHHS_VERSION)
endif (_GSHHS_FILE AND NOT GSHHS_FOUND)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (GSHHS DEFAULT_MSG
	GSHHS_PATH GSHHS_VERSION GSHHS_EXT)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
