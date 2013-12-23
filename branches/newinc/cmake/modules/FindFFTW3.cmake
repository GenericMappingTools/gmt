#
# $Id$
#
# Locate fftw3
#
# This module accepts the following environment variables:
#
#    FFTW3_DIR or FFTW3_ROOT - Specify the location of FFTW3
#
# This module defines the following CMake variables:
#
#    FFTW3_FOUND - True if libfftw3 is found
#    FFTW3F_LIBRARY - A variable pointing to the FFTW3 single precision library
#    FFTW3F_THREADS_LIBRARY - Threaded single precision library
#    FFTW3_INCLUDE_DIR - Where to find the headers

if (DEFINED FFTW3_ROOT AND NOT FFTW3_ROOT)
	set (FFTW3F_LIBRARY "" CACHE INTERNAL "")
	set (FFTW3F_THREADS_LIBRARY "" CACHE INTERNAL "")
	set (FFTW3_INCLUDE_DIR "" CACHE INTERNAL "")
	return()
endif()

find_path (FFTW3_INCLUDE_DIR fftw3.h
	HINTS
	${FFTW3_DIR}
	${FFTW3_ROOT}
	$ENV{FFTW3_DIR}
	$ENV{FFTW3_ROOT}
	PATH_SUFFIXES include
	PATHS
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt)

# single precision
find_library (FFTW3F_LIBRARY
	NAMES fftw3f
	HINTS
	${FFTW3_DIR}
	${FFTW3_ROOT}
	$ENV{FFTW3_DIR}
	$ENV{FFTW3_ROOT}
	PATH_SUFFIXES lib64 lib
	PATHS
	/sw
	/opt/local
	/opt/csw
	/opt)

get_filename_component (_fftw3_libpath ${FFTW3F_LIBRARY} PATH)

# threaded single precision
find_library (FFTW3F_THREADS_LIBRARY
	NAMES fftw3f_threads
	HINTS ${_fftw3_libpath})

if (FFTW3F_LIBRARY)
	# test if FFTW >= 3.3
	include (CMakePushCheckState)
	cmake_push_check_state() # save state of CMAKE_REQUIRED_*
	set (CMAKE_REQUIRED_INCLUDES ${FFTW3_INCLUDE_DIR})
	set (CMAKE_REQUIRED_LIBRARIES ${FFTW3F_LIBRARY})
	include (CheckSymbolExists)
	# this might fail with static FFTW3 library:
	check_symbol_exists (fftwf_import_wisdom_from_filename fftw3.h FFTW3F_VERSION_RECENT)
	cmake_pop_check_state() # restore state of CMAKE_REQUIRED_*

	# old version warning
	if (NOT FFTW3F_VERSION_RECENT)
		message (WARNING "FFTW library found, but it is either too old (<3.3) or statically-linked.")
		unset (FFTW3F_LIBRARY CACHE)
		unset (FFTW3F_THREADS_LIBRARY CACHE)
		unset (FFTW3_INCLUDE_DIR CACHE)
		unset (FFTW3F_VERSION_RECENT CACHE)
		return()
	endif (NOT FFTW3F_VERSION_RECENT)
endif (FFTW3F_LIBRARY)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (FFTW3 DEFAULT_MSG FFTW3F_LIBRARY FFTW3_INCLUDE_DIR FFTW3F_VERSION_RECENT)

set (FFTW3F_LIBRARIES ${FFTW3F_LIBRARY})
if (FFTW3F_THREADS_LIBRARY)
	list (APPEND FFTW3F_LIBRARIES ${FFTW3F_THREADS_LIBRARY})
endif (FFTW3F_THREADS_LIBRARY)
set (FFTW3_INCLUDE_DIRS ${FFTW3_INCLUDE_DIR})

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
