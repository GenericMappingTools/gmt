#
# Use this file to override variables in 'ConfigDefault.cmake' on a per-user basis.
# First copy 'ConfigUserDefault.cmake' to 'ConfigUser.cmake', then edit 'ConfigUser.cmake'.
# 'ConfigUser.cmake' is not version controlled (currently listed in svn:ignore property)
#
# $Revision$ 
# $LastChangedDate$
#

# Installation path:
#set(CMAKE_INSTALL_PREFIX "prefix_path")

# Set build type can be: empty, Debug, Release, relwithdebinfo or minsizerel (default is Release)
#set(CMAKE_BUILD_TYPE Debug)

# set location of netcdf (can be root directory, path to header file or path to nc-config):
#set(NETCDF_DIR "netcdf_install_prefix")

# set location of gdal (can be root directory, path to header file or path to gdal-config):
# set(GDAL_DIR "gdal_install_prefix")

# Enable compatibility mode:
#set(GMT_COMPAT "yes")

# Enable file locking:
#set(FLOCK "enabled")

# Use fast, non-GPL triangulation routine by J. Shewchuk:
#set(TRIANGLE_D "yes")

# Extra debugging for developers:
#add_definitions(-DDEBUG)

# Extra compiler Flags:
#set(CMAKE_C_FLAGS_DEBUG -ggdb3)
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

