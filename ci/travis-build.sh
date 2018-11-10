#!/bin/bash
# Build and install GMT

# To return a failure if any commands inside fail
set -e

cat >> cmake/ConfigUser.cmake << 'EOF'
set (CMAKE_INSTALL_PREFIX "$ENV{INSTALLDIR}")
set (GMT_LIBDIR "$ENV{INSTALLDIR}/lib")
set (DCW_ROOT "$ENV{COASTLINEDIR}")
set (GSHHG_ROOT "$ENV{COASTLINEDIR}")
enable_testing()
set (DO_EXAMPLES TRUE)
set (DO_TESTS TRUE)
set (DO_ANIMATIONS TRUE)
EOF

cat cmake/ConfigUser.cmake

mkdir build && cd build

cmake ..

make -j
make check
make install

# Turn off exit on failure.
set +e
