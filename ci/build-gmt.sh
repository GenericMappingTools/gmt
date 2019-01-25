#!/usr/bin/env bash
# Build and install GMT

# To return a failure if any commands inside fail
set -e

cat > cmake/ConfigUser.cmake << 'EOF'
set (CMAKE_INSTALL_PREFIX "$ENV{INSTALLDIR}")
set (DCW_ROOT "$ENV{COASTLINEDIR}")
set (GSHHG_ROOT "$ENV{COASTLINEDIR}")
set (CMAKE_C_FLAGS "-Wall -Wdeclaration-after-statement ${CMAKE_C_FLAGS}")
EOF

if [[ "$TEST" == "true" ]]; then
    cat >> cmake/ConfigUser.cmake << 'EOF'
enable_testing()
set (DO_EXAMPLES TRUE)
set (DO_TESTS TRUE)
set (N_TEST_JOBS 2)
set (CMAKE_C_FLAGS "-Wextra -coverage ${CMAKE_C_FLAGS}")
EOF
fi

if [[ "$TEST" == "true" || "$BUILD_DOCS" == "true" ]]; then
    cat >> cmake/ConfigUser.cmake << 'EOF'
set (DO_ANIMATIONS TRUE)
EOF
fi

echo ""
echo "Using the following cmake configuration:"
cat cmake/ConfigUser.cmake
echo ""

mkdir -p build && cd build

# Configure
cmake -G Ninja ..

# Show CMakeCache.txt, strip comments
grep -Ev "^(//|$)" CMakeCache.txt

# Build and install
cmake --build .
cmake --build . --target install

# Turn off exit on failure.
set +e
