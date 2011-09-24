# 
# $Id$
#
# To build out-of-source do (example):
#
#  md build && cd build
#  cmake -DCMAKE_INSTALL_PREFIX=gmt5 -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
#
# CMAKE_BUILD_TYPE can be: empty, Debug, Release, RelWithDebInfo or MinSizeRel
#
# cmake creates a new file cmake/ConfigUser.cmake if it does not already
# exist. You can configure aditional options there.
#

# make sure the user doesn't play dirty with symlinks
get_filename_component(srcdir "${CMAKE_SOURCE_DIR}" REALPATH)
get_filename_component(bindir "${CMAKE_BINARY_DIR}" REALPATH)

# disallow in-source builds
if(${srcdir} STREQUAL ${bindir})
  message(FATAL_ERROR "In-source builds are not allowed. "
  "Please create a directory and run cmake from there, passing the path "
  "to this source directory as the last argument. This process created "
  "the file `CMakeCache.txt' and the directory `CMakeFiles' in ${srcdir}. "
  "Please remove them.")
endif(${srcdir} STREQUAL ${bindir})


cmake_minimum_required( VERSION 2.8 )

project( GMT )

# Where to find our CMake modules (this variable is visible in subdirectories).
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

# Include configuration options (default options and options overridden by user).
include (ConfigCMake)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file(
  "${PROJECT_SOURCE_DIR}/config.h.cmake"
  "${PROJECT_BINARY_DIR}/config.h")


# gmt paths
set(GMT_DATAROOTDIR "${CMAKE_INSTALL_PREFIX}/share")
set(GMT_SHARE_PATH  "${GMT_DATAROOTDIR}")

#add_definitions(-DGMT_SHARE_PATH="${GMT_SHARE_PATH}")

# configure a header file to pass some of the CMake settings
# to the source code
configure_file(
  "${PROJECT_SOURCE_DIR}/config.h.cmake"
  "${PROJECT_BINARY_DIR}/config.h")


# add subtrees
add_subdirectory( src )
add_subdirectory( share )


# configuration done
message(
  "** Installing GMT in ${CMAKE_INSTALL_PREFIX}\n"
  "** GMT_SHARE_PATH: ${GMT_SHARE_PATH}\n"
  "** GDAL: ${HAVE_GDAL}\n"
  "** Triangulation method: ${TRIANGULATE}\n"
  "** File locking: ${FLOCK}"
  )
