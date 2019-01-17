#!/usr/bin/env bash
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
set (N_TEST_JOBS 2)
EOF

if [ "$COVERAGE" == "true" ]; then
    cat >> cmake/ConfigUser.cmake << 'EOF'
set (CMAKE_C_FLAGS "-Wall -Wdeclaration-after-statement -coverage -O0")
EOF
fi

cat cmake/ConfigUser.cmake

mkdir build && cd build

cmake ..

make -j
make install

# We are fixing the paths to dynamic library files inside library and binary
# files because something in 'make install' is doubling up the path to the
# library files. This only happens on OSX. Anyone who knows how to solve that
# problem is free to contact the maintainers.
if [[ "$TRAVIS_OS_NAME" == "osx" ]];then
    install_name_tool -id $INSTALLDIR/lib/libgmt.6.dylib $INSTALLDIR/lib/libgmt.6.dylib
    install_name_tool -id $INSTALLDIR/lib/libpostscriptlight.6.dylib $INSTALLDIR/lib/libpostscriptlight.6.dylib
    install_name_tool -change $INSTALLDIR/$INSTALLDIR/lib/libgmt.6.dylib $INSTALLDIR/lib/libgmt.6.dylib $INSTALLDIR/lib/gmt/plugins/supplements.so
    install_name_tool -change $INSTALLDIR/$INSTALLDIR/lib/libpostscriptlight.6.dylib $INSTALLDIR/lib/libpostscriptlight.6.dylib $INSTALLDIR/lib/gmt/plugins/supplements.so
    install_name_tool -change $INSTALLDIR/$INSTALLDIR/lib/libgmt.6.dylib $INSTALLDIR/lib/libgmt.6.dylib $INSTALLDIR/bin/gmt
    install_name_tool -change $INSTALLDIR/$INSTALLDIR/lib/libpostscriptlight.6.dylib $INSTALLDIR/lib/libpostscriptlight.6.dylib $INSTALLDIR/bin/gmt
    install_name_tool -change $INSTALLDIR/$INSTALLDIR/lib/libpostscriptlight.6.dylib $INSTALLDIR/lib/libpostscriptlight.6.dylib $INSTALLDIR/lib/libgmt.6.dylib
fi

# Turn off exit on failure.
set +e
