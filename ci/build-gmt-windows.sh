#!/usr/bin/env bash
# Build and install GMT

# To return a failure if any commands inside fail
set -e

cat > cmake/ConfigUser.cmake << 'EOF'
set (CMAKE_INSTALL_PREFIX "$ENV{INSTALLDIR}")
set (DCW_ROOT "$ENV{COASTLINEDIR}")
set (GSHHG_ROOT "$ENV{COASTLINEDIR}")
set (COPY_GSHHG true)
set (COPY_DCW true)
set (GMT_INSTALL_MODULE_LINKS FALSE)
set (CMAKE_C_FLAGS "/D_CRT_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_DEPRECATE ${CMAKE_C_FLAGS}")
set (CMAKE_C_FLAGS "/D_CRT_NONSTDC_NO_DEPRECATE /D_SCL_SECURE_NO_DEPRECATE ${CMAKE_C_FLAGS}")
EOF

if [[ "$TEST" == "true" ]]; then
    cat >> cmake/ConfigUser.cmake << 'EOF'
enable_testing()
set (DO_EXAMPLES TRUE)
set (DO_TESTS TRUE)
set (DO_API_TESTS ON)
set (SUPPORT_EXEC_IN_BINARY_DIR TRUE)
EOF
fi

if [[ "$BUILD_DOCS" == "true" ]]; then
    cat >> cmake/ConfigUser.cmake << 'EOF'
set (DO_ANIMATIONS TRUE)
set (CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")
EOF
fi

echo ""
echo "Using the following cmake configuration:"
cat cmake/ConfigUser.cmake
echo ""

# Turn off exit on failure.
set +e
