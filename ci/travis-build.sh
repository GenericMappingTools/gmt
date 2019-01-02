#!/bin/bash
# Build and install GMT

# To return a failure if any commands inside fail
set -e

cat > cmake/ConfigUser.cmake << 'EOF'
set (CMAKE_INSTALL_PREFIX "$ENV{INSTALLDIR}")
set (GMT_LIBDIR "$ENV{INSTALLDIR}/lib")
set (DCW_ROOT "$ENV{COASTLINEDIR}")
set (GSHHG_ROOT "$ENV{COASTLINEDIR}")

set (CMAKE_BUILD_TYPE Debug)
enable_testing()
set (DO_EXAMPLES TRUE)
set (DO_TESTS TRUE)
set (DO_ANIMATIONS TRUE)
EOF

if [ "$COVERAGE" == "true" ]; then
    cat >> cmake/ConfigUser.cmake << 'EOF'
set (CMAKE_BUILD_TYPE Debug)
set (CMAKE_C_FLAGS "-Wall -Wdeclaration-after-statement -coverage -O0")
EOF
fi

cat cmake/ConfigUser.cmake

mkdir build && cd build

cmake ..

make -j
make install

# Turn off exit on failure.
set +e
