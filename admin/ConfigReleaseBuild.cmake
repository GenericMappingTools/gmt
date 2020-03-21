#
# Use this file as ConfigUser.cmake when building the release
# Make sure GMT_GSHHG_SOURCE and GMT_DCW_SOURCE are defined in
# your environment and pointing to the latest releases.
#
#-------------------------------------------------------------
set (CMAKE_BUILD_TYPE Release)
set (CMAKE_INSTALL_PREFIX "gmt-${GMT_PACKAGE_VERSION}")
set (GSHHG_ROOT "$ENV{GMT_GSHHG_SOURCE}")
set (DCW_ROOT "$ENV{GMT_DCW_SOURCE}")

set (GMT_USE_THREADS TRUE)
set (GMT_ENABLE_OPENMP TRUE)
set (GMT_PUBLIC_RELEASE TRUE)

# recommended even for release build
set (CMAKE_C_FLAGS "-Wall -Wdeclaration-after-statement ${CMAKE_C_FLAGS}")
# extra warnings
set (CMAKE_C_FLAGS "-Wextra ${CMAKE_C_FLAGS}")
# Include all the external executables and shared libraries
# The add_macOS_cpack.txt is created by build-release.sh and placed in build
if (APPLE)
	set (EXTRA_INCLUDE_EXES "${CMAKE_BINARY_DIR}/add_macOS_cpack.txt")
endif (APPLE)
