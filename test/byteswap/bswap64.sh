#!/usr/bin/env bash
#

# test byteswapping 64 bit integers
exit 0 # not implemented yet, disabled for the time being

# generate random binary file
BYTES=$((1024*1024)) # 1 MiB
head -c $BYTES /dev/urandom > data.b

# these won't work because of GMT's internal double representation:
#gmt convert -bi1L -bo1Lw data.b | gmt convert -bi1Lw -bo1L > out.u
#gmt convert -bi1l -bo1lw data.b | gmt convert -bi1lw -bo1l > out.d

# swap using gmt xyz2grd
gmt xyz2grd -S8 -Gswapped_tmp.b data.b
gmt xyz2grd -S8 -Gswapped.b swapped_tmp.b

# swap skipping 16384 bytes
gmt xyz2grd -S8/16384 -Gswapped_tmp.b data.b
gmt xyz2grd -S8/16384 -Gswapped_skip.b swapped_tmp.b

# swap region of 524288 bytes
gmt xyz2grd -S8/16384/524288 -Gswapped_tmp.b data.b
gmt xyz2grd -S8/16384/524288 -Gswapped_region.b swapped_tmp.b

# compare result
diff -q data.b swapped.b
diff -q data.b swapped_skip.b
diff -q data.b swapped_region.b
