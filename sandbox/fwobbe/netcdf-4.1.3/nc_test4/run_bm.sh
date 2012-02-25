#!/bin/sh

# This shell runs a bunch of benchmarks on some specific files
# available at Unidata.

# $Id$

set -e

# Radar 2D file. Make sure we have a local disk copy. Not much point
# in benchmarking read and write times over NFS!
TMP=/shecky/data
d1=20070803-2300
echo "howdy!"
ls $TMP/${d1}-2d.nc3
if ! test -f $TMP/${d1}-2d.nc3; then
    cp /upc/share/testdata/nssl/mosaic2d_nc/tile1/$d1.netcdf $TMP/$d1-2d.nc3
fi

# Copy the 2D rarar file into a netCDF-4 version, with various
# compression settings.
out1=radar_2d_compression.txt
c0=100
c1=200
h=-h
for ((s=0; s < 2 ; s++))
do
    for ((d=0; d <= 9 ; d=d+2))
    do
	cmd="./bm_file $h -f 3 -o $TMP/$d1-2d.nc4 -c 0:${d}:${s}:${c0}:${c1}"
	for ((v=1; v < 12; v++))
	do
	    cmd="$cmd,${v}:${d}:${s}:${c0}:${c1}"
	done
	cmd="$cmd $TMP/$d1-2d.nc3"
	echo "cmd=$cmd"
 	if ! ($cmd >> $out1); then
 	    exit 1;
 	fi
	h=
    done
done

exit 0

# Get local copy of the radar 3D file.
d1=20070803-2300
if ! test -f $TMP/${d1}-3d.nc3; then
    cp /upc/share/testdata/nssl/mosaic3d_nc/tile1/20070803-2300.netcdf $TMP/${d1}-3d.nc3
fi

# Test diferent compressions, with and without shuffle.
out1=radar_3d_compression.txt
c0=3
c1=100
c2=200
h=-h
for ((s=0; s < 2 ; s++))
do
    for ((d=0; d <= 9 ; d++))
    do
	cmd="./bm_file $h -f 3 -o $TMP/$d1.nc4 -c 0:${d}:${s}:${c0}:${c1}:${c2} $TMP/$d1.nc3"
	echo "cmd=$cmd"
	if ! ($cmd >> $out1); then
	    exit 1;
	fi
	h=
    done
done

# Try different chunk sizes with the same compession.
out1=radar_3d_chunking.txt
s=1
d=3
h=-h
for c0 in 1 2 5
do
    for c1 in 10 100 200 500
    do
	for c3 in 10 100 200 500
	do
	    cmd="./bm_file $h -f 3 -o $TMP/$d1.nc4 -c 0:${d}:${s}:${c0}:${c1}:${c2} $TMP/$d1.nc3"
	    echo "cmd=$cmd"
	    if ! ($cmd >> $out1); then
		exit 1;
	    fi
	    h=
	done
    done
done
