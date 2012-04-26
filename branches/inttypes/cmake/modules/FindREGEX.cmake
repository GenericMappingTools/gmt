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
# - Locate a C-style regex library
# This module defines
#  REGEX_LIBRARY, the library to link against, if needed
#  REGEX_FOUND, if false, do not try to link to regex
#  REGEX_INCLUDE_DIR, where to find regex.h
#

set (REGEX_FOUND "NO")
include (CheckCSourceCompiles)
set (REGEX_LIBRARY)
find_path (REGEX_INCLUDE_DIR
  NAMES regex.h
  PATHS
  $ENV{REGEXDIR}/include
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
  /usr/include/awk
  /usr/local/include/awk)

#try compiling, even if not found
check_c_source_compiles ("int main() {(void)regcomp();}" REGCOMP_IN_LIBC)

if (REGEX_INCLUDE_DIR)
  if (NOT REGCOMP_IN_LIBC)
    # we need to link some library
    find_library (REGEX_LIBRARY_TEMP
      NAMES regex
      PATHS
      $ENV{REGEXDIR}/lib
      /usr/local/lib
      /usr/lib
      /sw/lib
      /opt/local/lib
      /opt/csw/lib
      /opt/lib)
    if (REGEX_LIBRARY_TEMP)
      set (CMAKE_REQUIRED_LIBRARIES ${REGEX_LIBRARY_TEMP})
      check_c_source_compiles ("int main() {(void)regcomp();}" REGCOMP_IN_REGEX)

      if (REGCOMP_IN_REGEX)
        set (REGEX_LIBRARY ${REGEX_LIBRARY_TEMP})
        set (REGEX_FOUND "YES")
      else (REGCOMP_IN_REGEX)
        message ("I found regex.h and a libregex but couldn't get regcomp() to compile")
      endif (REGCOMP_IN_REGEX)

    else (REGEX_LIBRARY_TEMP)
      message ("I found regex.h but regcomp() is not in libc or libregex")
    endif (REGEX_LIBRARY_TEMP)

  else (NOT REGCOMP_IN_LIBC)
    set (REGEX_FOUND "YES")
  endif (NOT REGCOMP_IN_LIBC)
else (REGEX_INCLUDE_DIR)
  if (REGCOMP_IN_LIBC)
    message ("regcomp() exists in libc, but I can't locate regex.h")
  endif (REGCOMP_IN_LIBC)
endif (REGEX_INCLUDE_DIR)

mark_as_advanced (REGEX_LIBRARY_TEMP REGCOMP_IN_LIBC REGCOMP_IN_REGEX)

