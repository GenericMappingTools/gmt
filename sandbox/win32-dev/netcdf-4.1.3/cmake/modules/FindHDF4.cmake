#
# To be used by projects that make use of CMakeified hdf-4.2.5
#

#
# Find the HDF4 includes and get all installed hdf4 library settings from
# HDF4-config.cmake file : Requires a CMake compatible hdf-4.2.6 or later 
# for this feature to work. The following vars are set if hdf4 is found.
#
# HDF4_FOUND               - True if found, otherwise all other vars are undefined
# HDF4_INCLUDE_DIR         - The include dir for main *.h files
# HDF4_FORTRAN_INCLUDE_DIR - The include dir for fortran modules and headers
# HDF4_VERSION_STRING      - full version (e.g. 4.2.5)
# HDF4_VERSION_MAJOR       - major part of version (e.g. 4.2)
# HDF4_VERSION_MINOR       - minor part (e.g. 5)
# 
# The following boolean vars will be defined
# HDF4_ENABLE_PARALLEL - 1 if HDF4 parallel supported
# HDF4_BUILD_FORTRAN   - 1 if HDF4 was compiled with fortran on
# HDF4_BUILD_CPP_LIB   - 1 if HDF4 was compiled with cpp on
# HDF4_BUILD_TOOLS     - 1 if HDF4 was compiled with tools on
# 
# Target names that are valid (depending on enabled options)
# will be the following
#
# hdf              : HDF4 C library
# hdf_f90cstub     : used by Fortran to C interface
# hdf_fortran      : Fortran HDF4 library
# mfhdf            : HDF4 multi-file C interface library
# xdr              : RPC library
# mfhdf_f90cstub   : used by Fortran to C interface to multi-file library
# mfhdf_fortran    : Fortran multi-file library
# 
# To aid in finding HDF4 as part of a subproject set
# HDF4_ROOT_DIR_HINT to the location where hdf4-config.cmake lies

INCLUDE (SelectLibraryConfigurations)
INCLUDE (FindPackageHandleStandardArgs)

# The HINTS option should only be used for values computed from the system.
SET (_HDF4_HINTS
    $ENV{HOME}/.local
    $ENV{HDF4_ROOT}
    $ENV{HDF4_ROOT_DIR_HINT}
)
# Hard-coded guesses should still go in PATHS. This ensures that the user
# environment can always override hard guesses.
SET (_HDF4_PATHS
    $ENV{HOME}/.local
    $ENV{HDF4_ROOT}
    $ENV{HDF4_ROOT_DIR_HINT}
    /usr/lib/hdf
    /usr/share/hdf
    /usr/local/hdf
    /usr/local/hdf/share
)

FIND_PATH (HDF4_ROOT_DIR "hdf4-config.cmake"
    HINTS ${_HDF4_HINTS}
    PATHS ${_HDF4_PATHS}
    PATH_SUFFIXES
        lib/cmake/hdf4
        share/cmake/hdf4
)

FIND_PATH (HDF4_INCLUDE_DIRS "hdf.h"
    HINTS ${_HDF4_HINTS}
    PATHS ${_HDF4_PATHS}
    PATH_SUFFIXES
        include
        Include
)

# For backwards compatibility we set HDF4_INCLUDE_DIR to the value of
# HDF4_INCLUDE_DIRS
SET ( HDF4_INCLUDE_DIR "${HDF4_INCLUDE_DIRS}" )

IF (HDF4_INCLUDE_DIR)
  INCLUDE (${HDF4_ROOT_DIR}/hdf4-config.cmake)
ENDIF (HDF4_INCLUDE_DIR)

# handle the QUIETLY and REQUIRED arguments and set HDF4_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(HDF4 REQUIRED_VARS HDF4_LIBRARIES HDF4_INCLUDE_DIRS)
