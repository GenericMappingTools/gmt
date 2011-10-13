#
# $Id$
#
# Locate GSHHS shorelines
#
# This module accepts the following environment variables:
#
#    GSHHS_ROOT - Specify the location of GSHHS
#
# This module defines the following CMake variables:
#
#    GSHHS_FOUND - True if libpcre is found
#    GSHHS_PATH  - A variable pointing to the GSHHS path

# get GSHHS path
find_path (GSHHS_PATH binned_GSHHS_c.cdf binned_GSHHS_l.cdf
	HINTS ${GSHHS_ROOT}
	PATHS ${CMAKE_SOURCE_DIR}
	PATH_SUFFIXES coast share/coast)

# get GSHHS file
if (GSHHS_PATH)
	find_file(_GSHHS_FILE binned_GSHHS_c.cdf binned_GSHHS_l.cdf
		HINTS ${GSHHS_PATH})
endif (GSHHS_PATH)

# get GSHHS version
if (_GSHHS_FILE)
	try_run(_EXIT_GSHHS_VERSION _COMPILED_GSHHS_VERSION
		${CMAKE_BINARY_DIR}/CMakeTmp
		${CMAKE_CURRENT_SOURCE_DIR}/gshhs_version.c
		CMAKE_FLAGS
		-DINCLUDE_DIRECTORIES=${NETCDF_INCLUDE_DIR}
		-DLINK_LIBRARIES=${NETCDF_LIBRARIES}
		#COMPILE_OUTPUT_VARIABLE _compile_debug
		RUN_OUTPUT_VARIABLE _GSHHS_VERSION_STRING
		ARGS ${_GSHHS_FILE})
	#message("_compile_debug>${_compile_debug}")
endif (_GSHHS_FILE)

# check version string
if (_COMPILED_GSHHS_VERSION AND _EXIT_GSHHS_VERSION EQUAL 0)
	# strip whitespace
	string(STRIP ${_GSHHS_VERSION_STRING} GSHHS_VERSION)
endif (_COMPILED_GSHHS_VERSION AND _EXIT_GSHHS_VERSION EQUAL 0)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (GSHHS DEFAULT_MSG GSHHS_PATH GSHHS_VERSION)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
