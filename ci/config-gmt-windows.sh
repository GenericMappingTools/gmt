#!/usr/bin/env bash
# Configure GMT settings under Windows

# To return a failure if any commands inside fail
set -e

cat > cmake/ConfigUser.cmake << 'EOF'
set (CMAKE_INSTALL_PREFIX "$ENV{INSTALLDIR}")
set (GSHHG_ROOT "$ENV{COASTLINEDIR}/gshhg")
set (DCW_ROOT "$ENV{COASTLINEDIR}/dcw")

set (GMT_ENABLE_OPENMP TRUE)
EOF

if [[ "$RUN_TESTS" == "true" ]]; then
    cat >> cmake/ConfigUser.cmake << 'EOF'
enable_testing()
set (DO_EXAMPLES TRUE)
set (DO_TESTS TRUE)
set (DO_API_TESTS ON)
set (SUPPORT_EXEC_IN_BINARY_DIR TRUE)

# Turn on testing of upcoming long-option syntax for common GMT options
add_definitions(-DUSE_COMMON_LONG_OPTIONS)
# Turn on testing of upcoming long-option syntax for module options
add_definitions(-DUSE_MODULE_LONG_OPTIONS)
EOF
fi

echo ""
echo "Using the following cmake configuration:"
cat cmake/ConfigUser.cmake
echo ""

# Turn off exit on failure.
set +e
