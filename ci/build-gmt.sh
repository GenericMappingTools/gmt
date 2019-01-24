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

if [ "$TEST" == "true" ]; then
    cat >> cmake/ConfigUser.cmake << 'EOF'
set (CMAKE_BUILD_TYPE Debug)
enable_testing()
set (DO_EXAMPLES TRUE)
set (DO_TESTS TRUE)
set (DO_ANIMATIONS TRUE)
set (N_TEST_JOBS 2)
set (CMAKE_C_FLAGS "-Wextra -coverage -O0 ${CMAKE_C_FLAGS}")
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

# We are fixing the paths to dynamic library files inside library and binary
# files because something in 'make install' is doubling up the path to the
# library files. This only happens on OSX. Anyone who knows how to solve that
# problem is free to contact the maintainers.
if 0; then # not needed anymore?  [[ "$TRAVIS_OS_NAME" == "osx" ]];then
    #install_name_tool -id $INSTALLDIR/lib/libgmt.6.dylib $INSTALLDIR/lib/libgmt.6.dylib
    #install_name_tool -id $INSTALLDIR/lib/libpostscriptlight.6.dylib $INSTALLDIR/lib/libpostscriptlight.6.dylib
    install_name_tool -change $INSTALLDIR/$INSTALLDIR/lib/libgmt.6.dylib $INSTALLDIR/lib/libgmt.6.dylib $INSTALLDIR/lib/gmt/plugins/supplements.so
    install_name_tool -change $INSTALLDIR/$INSTALLDIR/lib/libpostscriptlight.6.dylib $INSTALLDIR/lib/libpostscriptlight.6.dylib $INSTALLDIR/lib/gmt/plugins/supplements.so
    install_name_tool -change $INSTALLDIR/$INSTALLDIR/lib/libgmt.6.dylib $INSTALLDIR/lib/libgmt.6.dylib $INSTALLDIR/bin/gmt
    install_name_tool -change $INSTALLDIR/$INSTALLDIR/lib/libpostscriptlight.6.dylib $INSTALLDIR/lib/libpostscriptlight.6.dylib $INSTALLDIR/bin/gmt
    install_name_tool -change $INSTALLDIR/$INSTALLDIR/lib/libpostscriptlight.6.dylib $INSTALLDIR/lib/libpostscriptlight.6.dylib $INSTALLDIR/lib/libgmt.6.dylib
fi

# Turn off exit on failure.
set +e
