#!/bin/bash
#
# $Id$

# test byteswapping 32 bit integers

# generate random binary file
BYTES=$((1024*1024)) # 1 MiB
head -c $BYTES /dev/urandom > data.b

# swap
gmtconvert -bi1I -bo1Iw data.b | gmtconvert -bi1Iw -bo1I > out.u
gmtconvert -bi1i -bo1iw data.b | gmtconvert -bi1iw -bo1i > out.d

# compare result
diff -q data.b out.u
diff -q data.b out.d
