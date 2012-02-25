#!/bin/sh

# This shell runs a bunch of benchmarks on some specific files
# available at Unidata. If you want to run this shell, you need these
# data files.

# This script gets and benchmarks against some 2D radar data.

# $Id$

set -e

# Radar 2D file. Make sure we have a local disk copy. Not much point
# in benchmarking read and write times over NFS!
TMP=/shecky/data
d1=20070803-2300
file_num=0
for t in 1 2 4
do
    file=${d1}_tile${t}-2d.nc3
    in_file[$file_num]=$file
    let file_num=$file_num+1
    if ! test -f $TMP/$file; then
	echo "getting file: $file"
	cp -f /upc/share/testdata/nssl/mosaic2d_nc/tile${t}/$d1.netcdf.gz $TMP
	gunzip -f $TMP/$d1.netcdf.gz
	cp $d1.netcdf $TMP/$file
    fi
done
num_in_files=${#in_file[@]}

# Copy the 2D rarar file into a netCDF-4 version, with various
# compression settings.
out1=radar_2d_compression.csv
rm -rf $out1
c0=1001
c1=500
h=-h
file_num=0
for ((s=0; s < 2 ; s++))
do
    for ((d=-1; d <= 9 ; d++))
    do
	# Confuse the disk buffering by copying the file each time, so
	# always reading a new file.
	cp $TMP/${in_file[${file_num}]} $TMP/cp_${in_file[${file_num}]}

	cmd="./bm_file $h -f 3 -d -o $TMP/$d1-2d.nc4 -c 0:${d}:${s}:${c0}:${c1}"
	for ((v=1; v < 12; v++))
	do
	    cmd="$cmd,${v}:${d}:${s}:${c0}:${c1}"
	done
	cmd="$cmd $TMP/cp_${in_file[${file_num}]}"
	echo "cmd=$cmd"
 	if ! ($cmd >> $out1); then
 	    exit 1;
 	fi
	rm $TMP/cp_${in_file[${file_num}]}
	h=
	let file_num=$file_num+1
	test $file_num -eq $num_in_files && file_num=0
    done
done

exit 0
