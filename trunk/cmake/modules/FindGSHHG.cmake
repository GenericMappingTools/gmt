#
# $Id$
#
# Locate GSHHG shorelines
#
# This module accepts the following environment variables:
#
#    GSHHG_ROOT         - Specify the location of GSHHG
#
# This module defines the following CMake variables:
#
#    GSHHG_FOUND        - True if GSHHG is found
#    GSHHG_PATH         - A variable pointing to the GSHHG path
#    GSHHG_VERSION      - String of the GSHHG version found
#    GSHHG_MIN_REQUIRED - Struct of {major, minor, patch} version required
#    GSHHG_EXT          - GSHHG netCDF file extension

# get GSHHG path
find_path (GSHHG_PATH
	NAMES binned_GSHHS_c.nc binned_GSHHS_c.cdf
	HINTS ${GSHHG_ROOT}
	PATH_SUFFIXES
	gshhg-gmt-nc4
	share/gshhg-gmt-nc4
	coast
	share/coast
	share/gmt/coast
	PATHS
	${CMAKE_SOURCE_DIR}
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt)

# get GSHHG file
if (GSHHG_PATH)
	find_file (_GSHHG_FILE
		NAMES binned_GSHHS_c.cdf binned_GSHHS_c.nc
		HINTS ${GSHHG_PATH})
endif (GSHHG_PATH)

# check GSHHG version
set (MIN_REQUIRED_GSHHG_VERSION "${MIN_REQUIRED_GSHHG_VERSION_MAJOR}.${MIN_REQUIRED_GSHHG_VERSION_MINOR}.${MIN_REQUIRED_GSHHG_VERSION_PATCH}")
if (_GSHHG_FILE AND NOT GSHHG_FOUND)
	try_run (_EXIT_GSHHG_VERSION _COMPILED_GSHHG_VERSION
		${CMAKE_BINARY_DIR}/CMakeTmp
		${CMAKE_CURRENT_SOURCE_DIR}/gshhg_version.c
		CMAKE_FLAGS
		-DINCLUDE_DIRECTORIES=${NETCDF_INCLUDE_DIR}
		-DLINK_LIBRARIES=${NETCDF_LIBRARIES}
		COMPILE_DEFINITIONS -DSTANDALONE
		RUN_OUTPUT_VARIABLE _GSHHG_VERSION_STRING
		ARGS ${_GSHHG_FILE} ${MIN_REQUIRED_GSHHG_VERSION})

	# check version string
	if (_COMPILED_GSHHG_VERSION)
		# strip whitespace
		string (STRIP ${_GSHHG_VERSION_STRING} GSHHG_VERSION)
		if (_EXIT_GSHHG_VERSION EQUAL 0)
			# found GSHHG of required version or higher
			set (GSHHG_VERSION ${GSHHG_VERSION} CACHE INTERNAL "GSHHG version")
			get_filename_component (GSHHG_EXT ${_GSHHG_FILE} EXT CACHE)
		elseif (_EXIT_GSHHG_VERSION EQUAL -1)
			# found GSHHG but version is too old
			message (WARNING "GSHHG found but it is too old (${GSHHG_VERSION}). "
				"Need at least ${MIN_REQUIRED_GSHHG_VERSION}.")
		endif (_EXIT_GSHHG_VERSION EQUAL 0)
	endif (_COMPILED_GSHHG_VERSION)
endif (_GSHHG_FILE AND NOT GSHHG_FOUND)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (GSHHG DEFAULT_MSG
	GSHHG_PATH GSHHG_VERSION GSHHG_EXT)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
