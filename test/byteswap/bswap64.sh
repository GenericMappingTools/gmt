#!/bin/bash
#
# $Id$

# test byteswapping 64 bit integers

# generate random binary file
BYTES=$((1024*1024)) # 1 MiB
head -c $BYTES /dev/urandom > data.b

# these won't work because of GMT's internal double representation:
#gmtconvert -bi1L -bo1Lw data.b | gmtconvert -bi1Lw -bo1L > out.u
#gmtconvert -bi1l -bo1lw data.b | gmtconvert -bi1lw -bo1l > out.d

# swap using xyz2grd
xyz2grd -Sl -Gswapped_tmp.b data.b
xyz2grd -Sl -Gswapped.b swapped_tmp.b

# swap skipping 16384 bytes
xyz2grd -Sl16384 -Gswapped_tmp.b data.b
xyz2grd -Sl16384 -Gswapped_skip.b swapped_tmp.b

# swap region of 524288 bytes
xyz2grd -Sl16384/524288 -Gswapped_tmp.b data.b
xyz2grd -Sl16384/524288 -Gswapped_region.b swapped_tmp.b

# compare result
diff -q data.b swapped_again.b
diff -q data.b swapped_skip.b
diff -q data.b swapped_region.b
