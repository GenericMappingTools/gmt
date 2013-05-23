#
# $Id$
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
find_path (DCW_PATH
	NAMES dcw-gmt.nc
	HINTS ${DCW_ROOT}
	PATH_SUFFIXES
	dcw-gmt
	share/dcw-gmt
	share/gmt/dcw-gmt
	PATHS
	${CMAKE_SOURCE_DIR}
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt)

# get DCW file
if (DCW_PATH)
	find_file (_DCW_FILE
		NAMES dcw-gmt.nc
		HINTS ${DCW_PATH})
endif (DCW_PATH)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (DCW DEFAULT_MSG
	DCW_PATH)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
