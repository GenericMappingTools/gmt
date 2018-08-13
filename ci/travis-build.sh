#!/bin/bash
# Build and install GMT

# To return a failure if any commands inside fail
set -e

mkdir build && cd build

cmake -D CMAKE_INSTALL_PREFIX=$INSTALLDIR \
      -D GMT_LIBDIR=$INSTALLDIR/lib \
      -D DCW_ROOT=$COASTLINEDIR \
      -D GSHHG_ROOT=$COASTLINEDIR \
      ..

make -j
make check
make install

# Turn off exit on failure.
set +e
