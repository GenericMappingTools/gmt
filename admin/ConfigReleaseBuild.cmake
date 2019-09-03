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
set (COPY_GSHHG TRUE)
set (COPY_DCW TRUE)

set (GMT_INSTALL_MODULE_LINKS FALSE)
set (GMT_USE_THREADS TRUE)
set (GMT_ENABLE_OPENMP TRUE)

# recommended even for release build
set (CMAKE_C_FLAGS "-Wall -Wdeclaration-after-statement ${CMAKE_C_FLAGS}")
# extra warnings
set (CMAKE_C_FLAGS "-Wextra ${CMAKE_C_FLAGS}")
