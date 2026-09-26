#!/usr/bin/env bash
# Configure GMT settings under Linux/macOS/Windows

# To return a failure if any commands inside fail
set -e

cat > cmake/ConfigUser.cmake << 'EOF'
set (CMAKE_INSTALL_PREFIX "$ENV{INSTALLDIR}")
set (GSHHG_ROOT "$ENV{COASTLINEDIR}/gshhg")
set (DCW_ROOT "$ENV{COASTLINEDIR}/dcw")

set (GMT_ENABLE_OPENMP TRUE)
EOF

# macOS: Set OpenMP_ROOT so that CMake can find the libomp header and library.
if [[ "$RUNNER_OS" == "macOS" ]]; then
    echo "set (OpenMP_ROOT $(brew --prefix)/opt/libomp/)" >> cmake/ConfigUser.cmake
fi

# Add GCC/Clang-specific compiler flags and threads settings
if [[ "$RUNNER_OS" != "Windows" ]]; then
    cat >> cmake/ConfigUser.cmake << 'EOF'

# recommended even for release build
set (CMAKE_C_FLAGS "-Wall -Wdeclaration-after-statement ${CMAKE_C_FLAGS}")
# extra warnings
set (CMAKE_C_FLAGS "-Wextra ${CMAKE_C_FLAGS}")
EOF

    if [[ "$EXCLUDE_OPTIONAL" == "true" ]]; then
        echo 'set (GMT_USE_THREADS TRUE)' >> cmake/ConfigUser.cmake
    else
        echo 'set (GMT_USE_THREADS FALSE)' >> cmake/ConfigUser.cmake
    fi

fi

# Settings related to tests.
if [[ "$RUN_TESTS" == "true" ]]; then
    cat >> cmake/ConfigUser.cmake << 'EOF'

enable_testing()
set (DO_EXAMPLES TRUE)
set (DO_TESTS TRUE)
set (DO_API_TESTS ON)
set (DO_SUPPLEMENT_TESTS ON)
set (SUPPORT_EXEC_IN_BINARY_DIR TRUE)
EOF

    # Debug build and code coverage are only used on Linux/macOS
    if [[ "$RUNNER_OS" != "Windows" ]]; then
        echo 'set (CMAKE_BUILD_TYPE Debug)' >> cmake/ConfigUser.cmake
        # For code coverage
        echo 'set (CMAKE_C_FLAGS "--coverage -O0 ${CMAKE_C_FLAGS}")' >> cmake/ConfigUser.cmake
    fi
fi

echo ""
echo "Using the following cmake configuration:"
cat cmake/ConfigUser.cmake
echo ""

# Turn off exit on failure.
set +e
