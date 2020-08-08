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

# recommended even for release build
set (CMAKE_C_FLAGS "-Wall -Wdeclaration-after-statement ${CMAKE_C_FLAGS}")
# extra warnings
set (CMAKE_C_FLAGS "-Wextra ${CMAKE_C_FLAGS}")
EOF

if [[ "$RUN_TESTS" == "true" ]]; then
    cat >> cmake/ConfigUser.cmake << 'EOF'
set (CMAKE_BUILD_TYPE Debug)

enable_testing()
set (DO_EXAMPLES TRUE)
set (DO_TESTS TRUE)
set (DO_API_TESTS ON)
set (SUPPORT_EXEC_IN_BINARY_DIR TRUE)

# For code coverage
set (CMAKE_C_FLAGS "-coverage -O0 ${CMAKE_C_FLAGS}")

# Turn on testing of upcoming long-option syntax for common GMT options
add_definitions(-DUSE_COMMON_LONG_OPTIONS)
# Turn on testing of upcoming long-option syntax for module options
add_definitions(-DUSE_MODULE_LONG_OPTIONS)
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
