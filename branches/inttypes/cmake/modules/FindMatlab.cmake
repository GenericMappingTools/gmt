#
# $Id$
#
# - this module looks for Matlab
# Defines:
#  MATLAB_INCLUDE_DIR: include path for mex.h, engine.h
#  MATLAB_LIBRARIES:   required libraries: libmex, etc
#  MATLAB_MEX_LIBRARY: path to libmex.lib
#  MATLAB_MX_LIBRARY:  path to libmx.lib
#  MATLAB_ENG_LIBRARY: path to libeng.lib
#
#  MATLAB_ROOT:        indicate path to Matlab
#
#=============================================================================
# Copyright 2005-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See COPYING-CMAKE-SCRIPTS for more information.
#=============================================================================

if (CMAKE_SIZEOF_VOID_P EQUAL 4)
	# x86
	set (_bin_path_suffixes
		bin/glnx86    # unix i386
		bin/maci)     # maxosx i386
else (CMAKE_SIZEOF_VOID_P EQUAL 4)
	# x86_64
	set (_bin_path_suffixes
		bin/glnxa64   # unix x86_64
		bin/maci64)   # macosx x86_64
endif (CMAKE_SIZEOF_VOID_P EQUAL 4)

set (_matlab_root_hardcoded
	/Applications/MATLAB_R2010a.app
	/Applications/MATLAB_R2009b.app
	/usr/local/matlab-7sp1
	/opt/matlab-7sp1
	$ENV{HOME}/matlab-7sp1
	$ENV{HOME}/redhat-matlab)

#set (MATLAB_FOUND FALSE)
if (WIN32)
	if (${CMAKE_GENERATOR} MATCHES "Visual Studio 6")
		set (MATLAB_ROOT "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MathWorks\\MATLAB\\7.0;MATLABROOT]/extern/lib/win32/microsoft/msvc60")
	else (${CMAKE_GENERATOR} MATCHES "Visual Studio 6")
		if (${CMAKE_GENERATOR} MATCHES "Visual Studio 7")
			# Assume people are generally using 7.1,
			# if using 7.0 need to link to: ../extern/lib/win32/microsoft/msvc70
			set (MATLAB_ROOT "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MathWorks\\MATLAB\\7.0;MATLABROOT]/extern/lib/win32/microsoft/msvc71")
		else (${CMAKE_GENERATOR} MATCHES "Visual Studio 7")
			if (${CMAKE_GENERATOR} MATCHES "Borland")
				# Same here, there are also: bcc50 and bcc51 directories
				set (MATLAB_ROOT "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MathWorks\\MATLAB\\7.0;MATLABROOT]/extern/lib/win32/microsoft/bcc54")
			else (${CMAKE_GENERATOR} MATCHES "Borland")
				if (MATLAB_FIND_REQUIRED)
					message (FATAL_ERROR "Generator not compatible: ${CMAKE_GENERATOR}")
				endif (MATLAB_FIND_REQUIRED)
			endif (${CMAKE_GENERATOR} MATCHES "Borland")
		endif (${CMAKE_GENERATOR} MATCHES "Visual Studio 7")
	endif (${CMAKE_GENERATOR} MATCHES "Visual Studio 6")
	find_library (MATLAB_MEX_LIBRARY
		libmex
		HINTS
		${MATLAB_ROOT})
	find_library (MATLAB_MX_LIBRARY
		libmx
		HINTS
		${MATLAB_ROOT})
	find_library (MATLAB_ENG_LIBRARY
		libeng
		HINTS
		${MATLAB_ROOT})
	find_path (MATLAB_INCLUDE_DIR
		"mex.h"
		HINTS
		"[HKEY_LOCAL_MACHINE\\SOFTWARE\\MathWorks\\MATLAB\\7.0;MATLABROOT]/extern/include")
else ( WIN32 )
	find_library (MATLAB_MEX_LIBRARY mex
		HINTS
		${MATLAB_ROOT}
		PATH_SUFFIXES
		${_bin_path_suffixes}
		PATHS
		${_matlab_root_hardcoded})
	find_library (MATLAB_MX_LIBRARY mx
		HINTS
		${MATLAB_ROOT}
		PATH_SUFFIXES
		${_bin_path_suffixes}
		PATHS
		${_matlab_root_hardcoded})
	find_library (MATLAB_ENG_LIBRARY eng
		HINTS
		${MATLAB_ROOT}
		PATH_SUFFIXES
		${_bin_path_suffixes}
		PATHS
		${_matlab_root_hardcoded})
	find_path (MATLAB_INCLUDE_DIR mex.h
		HINTS
		${MATLAB_ROOT}
		PATH_SUFFIXES
		extern/include
		PATHS
		${_matlab_root_hardcoded})
endif (WIN32)

# This is common to UNIX and Win32:
set (MATLAB_LIBRARIES
	${MATLAB_MEX_LIBRARY}
	${MATLAB_MX_LIBRARY}
	${MATLAB_ENG_LIBRARY}
	)

mark_as_advanced (
	MATLAB_LIBRARIES
	MATLAB_MEX_LIBRARY
	MATLAB_MX_LIBRARY
	MATLAB_ENG_LIBRARY
	MATLAB_INCLUDE_DIR
	MATLAB_FOUND
	MATLAB_ROOT
	)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (MATLAB DEFAULT_MSG MATLAB_LIBRARIES MATLAB_INCLUDE_DIR)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
