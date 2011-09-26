#
# Use this file to override variables in 'ConfigDefault.cmake' on a per-user basis.
# First copy 'ConfigUserDefault.cmake' to 'ConfigUser.cmake', then edit 'ConfigUser.cmake'.
# 'ConfigUser.cmake' is not version controlled (currently listed in svn:ignore property)
#
# $Revision$ 
# $LastChangedDate$
#

#
# Note: CMake considers an empty string, "FALSE", "OFF", "NO", or any string ending
# in "-NOTFOUND" to be false. (This happens to be case-insensitive, so "False",
# "off", "no", and "something-NotFound" are all false.) Other values are true. Thus
# it matters not whether you use TRUE and FALSE, ON and OFF, or YES and NO for your
# booleans.
#

# Installation path [auto]:
#set(CMAKE_INSTALL_PREFIX "prefix_path")

# Set build type can be: empty, Debug, Release, relwithdebinfo or minsizerel [Release]:
#set(CMAKE_BUILD_TYPE Debug)

# Set location of NetCDF (can be root directory, path to header file or path to nc-config) [auto]:
#set(NETCDF_DIR "netcdf_install_prefix")

# Set location of GDAL (can be root directory, path to header file or path to gdal-config) [auto]:
#set(GDAL_DIR "gdal_install_prefix")

# Enable compatibility mode [FALSE]:
#set(GMT_COMPAT yes)

# Enable file locking [FALSE]:
#set(FLOCK yes)

# Use fast, non-GPL triangulation routine by J. Shewchuk [FALSE]:
#set(TRIANGLE_D yes)

# Extra debugging for developers:
#add_definitions(-DDEBUG)
#set(CMAKE_C_FLAGS_DEBUG -ggdb3)
#set(CMAKE_C_FLAGS_RELEASE -ggdb3)

# Extra compiler Flags:
#add_definitions(
#  -Wall
#  -Wdeclaration-after-statement
#  -fPIC
#  -fno-strict-aliasing
#  -Wextra
#  -Wuninitialized
#  )

# This is for gcc on Solaris to avoid "relocations remain against
# allocatable but non-writable sections" problems:
#set(USER_GMTLIB_LINK_FLAGS -mimpure-text)

