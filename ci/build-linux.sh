#!/bin/bash
# Build and install GMT

mkdir $INSTALLDIR

mkdir build && cd build

cmake -D CMAKE_INSTALL_INSTALLDIR=$INSTALLDIR \
      -D GMT_LIBDIR=$INSTALLDIR/lib \
      -D DCW_ROOT=$COASTLINEDIR \
      -D GSHHG_ROOT=$COASTLINEDIR \
      ..

make
make check
make install
