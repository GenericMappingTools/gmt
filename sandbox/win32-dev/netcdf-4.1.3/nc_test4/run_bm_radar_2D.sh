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
# CHUNKING settings.
out1=radar_2d_chunking.csv
out2=radar_2d_chunking_2.csv
rm -rf $out1 $out2

# Turn on header (for the first run of bm_file).
h=-h

# Turn off compression and shuffle filters.
s=0
d=-1

# file_num=0
# for c0 in 251 1001 1501
# do
#     for c1 in 251 1001 2001
#     do
# 	# Build the command including chunk sizes for all 13 vars.
# 	cmd="./bm_file $h -f 4 -o $TMP/$d1-2d_${c0}x${c1}.nc4 -c 0:${d}:${s}:${c0}:${c1}"
# 	for ((v=1; v < 12; v++))
# 	do
# 	    cmd="$cmd,${v}:${d}:${s}:${c0}:${c1}"
# 	done
# 	cmd="$cmd $TMP/${in_file[${file_num}]}"
# 	echo "cmd=$cmd"
#  	if ! ($cmd); then
#  	    exit 1;
#  	fi
# 	h=
	
# 	# Switch to the next input file of three.
# 	let file_num=$file_num+1
# 	test $file_num -eq $num_in_files && file_num=0
#     done
# done


file_num=0
for c0 in 251 1001 1501
do
    for c1 in 251 1001 2001
    do
	for try in 0 1 2 3 4 5 6 7 8 9
	do
	# Confuse the disk buffering by copying the file each time, so
	# always reading a new file.
#	    cp $TMP/${in_file[${file_num}]} $TMP/cp_${in_file[${file_num}]}

	# Build the command including chunk sizes for all 13 vars.
	    cmd="./bm_file $h -f 4 -c 0:${d}:${s}:${c0}:${c1}"
	    for ((v=1; v < 12; v++))
	    do
		cmd="$cmd,${v}:${d}:${s}:${c0}:${c1}"
	    done
	    cmd="$cmd $TMP/${in_file[${file_num}]}"
	    echo "cmd=$cmd"
	    sudo bash ./clear_cache.sh
 	    if ! ($cmd >> $out1); then
 		exit 1;
 	    fi

	    cmd="./bm_file $h -f 3 -c 0:${d}:${s}:${c0}:${c1}"
	    for ((v=1; v < 12; v++))
	    do
		cmd="$cmd,${v}:${d}:${s}:${c0}:${c1}"
	    done
	    cmd="$cmd $TMP/$d1-2d_${c0}x${c1}.nc4"
	    echo "cmd=$cmd"
	    sudo bash ./clear_cache.sh
 	    if ! ($cmd >> $out2); then
 		exit 1;
 	    fi

	# Remove the copy. Next read will a "new" file.
	#    rm $TMP/cp_${in_file[${file_num}]}

	# Turn off header next time around.
	    h=

	# Switch to the next input file of three.
	    let file_num=$file_num+1
	    test $file_num -eq $num_in_files && file_num=0
	done
    done
done

exit 0
