#
# $Id$
#
# - Find regex 
# Find the native REGEX includes and library
#
#  REGEX_INCLUDE_DIR   - where to find regex.h, etc.
#  REGEX_LIBRARY       - List of libraries when using regex.
#  REGEX_FOUND         - True if regex found.
#
#=============================================================================

include (CheckLibraryExists)

# because at least one specific framework (Ruby) on OSX has been observed
# to include its own regex.h copy, check frameworks last - /usr/include
# is preferred to a package-specific copy for a generic regex search
set (CMAKE_FIND_FRAMEWORK LAST)
find_path (REGEX_INCLUDE_DIR regex.h)
set (REGEX_NAMES c regex compat)
foreach (rname ${REGEX_NAMES})
	if (NOT REGEX_LIBRARY)
		check_library_exists (${rname} regcomp "" HAVE_REGEX_LIB)
		if (HAVE_REGEX_LIB)
			find_library (REGEX_LIBRARY NAMES ${rname})
		endif (HAVE_REGEX_LIB)
	endif (NOT REGEX_LIBRARY)
endforeach (rname ${REGEX_NAMES})

mark_as_advanced(REGEX_LIBRARY REGEX_INCLUDE_DIR)

# handle the QUIETLY and REQUIRED arguments and set REGEX_FOUND to TRUE if 
# all listed variables are TRUE
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (REGEX DEFAULT_MSG REGEX_INCLUDE_DIR REGEX_LIBRARY)
