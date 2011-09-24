#
#	Information for building GMT 5 with CMake
#
#	$Id$
#

#
# Quick start:
#

svn switch to cmake-build and cd into tree:

 mkdir build && cd build
 cmake -DCMAKE_INSTALL_PREFIX=gmt5 -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
 make -j8

Will build out-of-source in the the directory build.

 make install
 
Installs a basic gmt in build/gmt5

#
# In more detail:
#

See http://www.cmake.org/cmake/help/cmake-2-8-docs.html

CMAKE_BUILD_TYPE can be: empty, Debug, Release, RelWithDebInfo or MinSizeRel

cmake makes a copy of cmake/ConfigUserTemplate.cmake and creates a new file
cmake/ConfigUser.cmake if it does not already exist. You can configure aditional
options there.

There are two configuration files:
1) "ConfigDefault.cmake" - is version controlled and used to add new default
    variables and set defaults for everyone.
2) "ConfigUser.cmake" - is not version controlled (currently listed in svn:ignore
    property) and used to override defaults on a per-user basis.

NOTE: If you want to change CMake behaviour just for yourself then modify the
      "ConfigUser.cmake" file (not "ConfigDefault.cmake").


#
# TODO list
#

* replace shell scripts, grep, sed, awk, etc. by cmake macros (this
  is necessary for cross platform compatibility)

* clean up config.h.cmake gmt_version.h.cmake gmt_notposix.h.cmake
   pslconfig.h.cmake

* make exported cmake variables consistent GMT_...

* add further subtrees

* build targets for manpages, examples, doc, tests

