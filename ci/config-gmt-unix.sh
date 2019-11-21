#!/usr/bin/env bash
# Build and install GMT

# To return a failure if any commands inside fail
set -e

cat > cmake/ConfigUser.cmake << 'EOF'
set (CMAKE_BUILD_TYPE Release)
set (CMAKE_INSTALL_PREFIX "$ENV{INSTALLDIR}")
set (GSHHG_ROOT "$ENV{COASTLINEDIR}/gshhg")
set (DCW_ROOT "$ENV{COASTLINEDIR}/dcw")
set (COPY_GSHHG TRUE)
set (COPY_DCW TRUE)

set (GMT_USE_THREADS TRUE)
set (GMT_ENABLE_OPENMP TRUE)

# recommended even for release build
set (CMAKE_C_FLAGS "-Wall -Wdeclaration-after-statement ${CMAKE_C_FLAGS}")
# extra warnings
set (CMAKE_C_FLAGS "-Wextra ${CMAKE_C_FLAGS}")
EOF

if [[ "$TEST" == "true" ]]; then
    cat >> cmake/ConfigUser.cmake << 'EOF'
enable_testing()
set (DO_EXAMPLES TRUE)
set (DO_TESTS TRUE)
set (DO_API_TESTS ON)
set (N_TEST_JOBS 2)
set (SUPPORT_EXEC_IN_BINARY_DIR TRUE)
set (CMAKE_C_FLAGS "-coverage -O0 ${CMAKE_C_FLAGS}")
EOF
fi

if [[ "$BUILD_DOCS" == "true" ]]; then
    cat >> cmake/ConfigUser.cmake << 'EOF'
set (DO_ANIMATIONS TRUE)
EOF
fi

echo ""
echo "Using the following cmake configuration:"
cat cmake/ConfigUser.cmake
echo ""

# Turn off exit on failure.
set +e
