#!/bin/bash
#
# $Id$

# test byteswapping 16 bit integers

# generate random binary file
BYTES=$((1024*1024)) # 1 MiB
head -c $BYTES /dev/urandom > data.b

# swap
gmtconvert -bi1H -bo1Hw data.b | gmtconvert -bi1Hw -bo1H > out.u
gmtconvert -bi1h -bo1hw data.b | gmtconvert -bi1hw -bo1h > out.d

# compare result
diff -q data.b out.u
diff -q data.b out.d

# swap using xyz2grd
xyz2grd -Sswapped_tmp.b -ZH data.b
xyz2grd -Sswapped.b -ZH swapped_tmp.b

# compare result
diff -q data.b swapped.b

# files must differ
if diff -q swapped_tmp.b swapped.b; then
  false # return with error
fi
