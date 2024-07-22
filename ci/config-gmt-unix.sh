#!/usr/bin/env bash
# Configure GMT setting under UNIX

# To return a failure if any commands inside fail
set -e

cat > cmake/ConfigUser.cmake << 'EOF'
set (CMAKE_INSTALL_PREFIX "$ENV{INSTALLDIR}")
set (GSHHG_ROOT "$ENV{COASTLINEDIR}/gshhg")
set (DCW_ROOT "$ENV{COASTLINEDIR}/dcw")

set (GMT_USE_THREADS TRUE)
set (GMT_ENABLE_OPENMP TRUE)

# Always use the 'static' data server in CI.
set (GMT_DATA_SERVER static)

# recommended even for release build
set (CMAKE_C_FLAGS "-Wall -Wdeclaration-after-statement ${CMAKE_C_FLAGS}")
# extra warnings
set (CMAKE_C_FLAGS "-Wextra ${CMAKE_C_FLAGS}")
EOF

# Set OpenMP_ROOT so that CMake can find the libomp header and library on macOS
if [[ "$RUNNER_OS" == "macOS" ]]; then
    echo "set (OpenMP_ROOT $(brew --prefix)/opt/libomp/)" >> cmake/ConfigUser.cmake
fi

if [[ "$RUN_TESTS" == "true" ]]; then
    cat >> cmake/ConfigUser.cmake << 'EOF'
set (CMAKE_BUILD_TYPE Debug)

enable_testing()
set (DO_EXAMPLES TRUE)
set (DO_TESTS TRUE)
set (DO_API_TESTS ON)
set (DO_SUPPLEMENT_TESTS ON)
set (SUPPORT_EXEC_IN_BINARY_DIR TRUE)

# For code coverage
set (CMAKE_C_FLAGS "--coverage -O0 ${CMAKE_C_FLAGS}")
EOF
fi

echo ""
echo "Using the following cmake configuration:"
cat cmake/ConfigUser.cmake
echo ""

# Turn off exit on failure.
set +e
