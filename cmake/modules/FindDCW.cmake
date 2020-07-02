#
#
# Locate DCW Digital Chart of the World for GMT
#
# This module accepts the following environment variables:
#
#    DCW_ROOT         - Specify the location of DCW
#
# This module defines the following CMake variables:
#
#    DCW_FOUND        - True if DCW is found
#    DCW_PATH         - A variable pointing to the DCW path

# get DCW path
find_path (DCW_PATH dcw-gmt.nc
	HINTS ${DCW_ROOT} $ENV{DCW_ROOT}
	PATH_SUFFIXES
	gmt-dcw
	dcw-gmt
	share/gmt/dcw
	share/gmt-dcw
	share/dcw-gmt
	PATHS
	${CMAKE_SOURCE_DIR}
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
	/usr/local
	DOC "Digital Chart of the World"
)

if (DCW_PATH)
	find_file (_DCW_FILE
		NAMES dcw-gmt.nc
		HINTS ${DCW_PATH})
endif (DCW_PATH)

# check dcw version via running gshhg_version
if (_DCW_FILE AND NOT DCW_FOUND)
	try_run (_EXIT_GSHHG_VERSION _COMPILED_GSHHG_VERSION
		${CMAKE_BINARY_DIR}/CMakeTmp
		${CMAKE_CURRENT_SOURCE_DIR}/gshhg_version.c
		CMAKE_FLAGS
		-DINCLUDE_DIRECTORIES=${NETCDF_INCLUDE_DIR}
		-DLINK_LIBRARIES=${NETCDF_LIBRARIES}
		COMPILE_DEFINITIONS -DSTANDALONE
		COMPILE_OUTPUT_VARIABLE _GSHHG_VERSION_COMPILE_OUT
		RUN_OUTPUT_VARIABLE _DCW_VERSION_STRING
		ARGS \"${_DCW_FILE}\")

	if (NOT _COMPILED_GSHHG_VERSION OR _EXIT_GSHHG_VERSION STREQUAL FAILED_TO_RUN)
		message(FATAL_ERROR "Cannot determine DCW version:\n
		${_GSHHG_VERSION_COMPILE_OUT}\n
		${_DCW_VERSION_STRING}")
	endif ()

	# check version string
	if (_COMPILED_GSHHG_VERSION)
		# strip whitespace
		string (STRIP ${_DCW_VERSION_STRING} DCW_VERSION)
		if (_EXIT_GSHHG_VERSION EQUAL 0)
			set (DCW_VERSION ${DCW_VERSION} CACHE INTERNAL "DCW version")
		endif (_EXIT_GSHHG_VERSION EQUAL 0)
	endif (_COMPILED_GSHHG_VERSION)
endif (_DCW_FILE AND NOT DCW_FOUND)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (DCW DEFAULT_MSG DCW_PATH)
