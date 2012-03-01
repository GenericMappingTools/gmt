#!/bin/sh
cmp nctest_classic.nc $srcdir/ref_nctest_classic.nc && cmp nctest_64bit_offset.nc $srcdir/ref_nctest_64bit_offset.nc

