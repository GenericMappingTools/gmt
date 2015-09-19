#
# $Id$
#
# Locate GSHHG shorelines
#
# This module accepts the following environment variables:
#
#  GSHHG_ROOT, GSHHGDIR - Specify the location of GSHHG
#
# This module defines the following CMake variables:
#
#    GSHHG_FOUND        - True if GSHHG is found
#    GSHHG_PATH         - A variable pointing to the GSHHG path
#    GSHHG_VERSION      - String of the GSHHG version found
#    GSHHG_MIN_REQUIRED_VERSION_{MAJOR, MINOR, PATCH}
#                       - Major, minor, and patch version required

# get GSHHG path
find_path (GSHHG_PATH
	NAMES binned_GSHHS_c.nc
	HINTS ${GSHHG_ROOT} $ENV{GSHHG_ROOT} $ENV{GSHHGDIR}
	PATH_SUFFIXES
	gmt-gshhg
	gshhg-gmt-nc4
	share/gmt/gshhg
	share/gmt-gshhg
	share/gshhg
	share/gshhg-gmt-nc4
	PATHS
	${CMAKE_SOURCE_DIR}
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
	/usr/local
	DOC "Global Self-consistent Hierarchical High-resolution Geography"
)

# get GSHHG file
if (GSHHG_PATH)
	find_file (_GSHHG_FILE
		NAMES binned_GSHHS_c.nc
		HINTS ${GSHHG_PATH})
endif (GSHHG_PATH)

# The minimum required GSHHG version
set (GSHHG_MIN_REQUIRED_VERSION_MAJOR 2 CACHE INTERNAL "GSHHG required version major")
set (GSHHG_MIN_REQUIRED_VERSION_MINOR 2 CACHE INTERNAL "GSHHG required version minor")
set (GSHHG_MIN_REQUIRED_VERSION_PATCH 0 CACHE INTERNAL "GSHHG required version patch")
set (GSHHG_MIN_REQUIRED_VERSION
	"${GSHHG_MIN_REQUIRED_VERSION_MAJOR}.${GSHHG_MIN_REQUIRED_VERSION_MINOR}.${GSHHG_MIN_REQUIRED_VERSION_PATCH}")

# check GSHHG version
if (_GSHHG_FILE AND NOT GSHHG_FOUND)
	try_run (_EXIT_GSHHG_VERSION _COMPILED_GSHHG_VERSION
		${CMAKE_BINARY_DIR}/CMakeTmp
		${CMAKE_CURRENT_SOURCE_DIR}/gshhg_version.c
		CMAKE_FLAGS
		-DINCLUDE_DIRECTORIES=${NETCDF_INCLUDE_DIR}
		-DLINK_LIBRARIES=${NETCDF_LIBRARIES}
		COMPILE_DEFINITIONS -DSTANDALONE
		COMPILE_OUTPUT_VARIABLE _GSHHG_VERSION_COMPILE_OUT
		RUN_OUTPUT_VARIABLE _GSHHG_VERSION_STRING
		ARGS \"${_GSHHG_FILE}\" ${GSHHG_MIN_REQUIRED_VERSION})

	if (NOT _COMPILED_GSHHG_VERSION OR _EXIT_GSHHG_VERSION STREQUAL FAILED_TO_RUN)
		message(FATAL_ERROR "Cannot determine GSHHG version:\n
		${_GSHHG_VERSION_COMPILE_OUT}\n
		${_GSHHG_VERSION_STRING}")
	endif ()

	# check version string
	if (_COMPILED_GSHHG_VERSION)
		# strip whitespace
		string (STRIP ${_GSHHG_VERSION_STRING} GSHHG_VERSION)
		if (_EXIT_GSHHG_VERSION EQUAL 0)
			# found GSHHG of required version or higher
			set (GSHHG_VERSION ${GSHHG_VERSION} CACHE INTERNAL "GSHHG version")
		elseif (_EXIT_GSHHG_VERSION EQUAL -1)
			# found GSHHG but version is too old
			message (WARNING "GSHHG found but it is too old (${GSHHG_VERSION}). "
				"Need at least ${GSHHG_MIN_REQUIRED_VERSION}.")
		endif (_EXIT_GSHHG_VERSION EQUAL 0)
	endif (_COMPILED_GSHHG_VERSION)
endif (_GSHHG_FILE AND NOT GSHHG_FOUND)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (GSHHG DEFAULT_MSG
	GSHHG_PATH GSHHG_VERSION)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
