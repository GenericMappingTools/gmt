#!/bin/sh

# This shell runs a bunch of benchmarks on some specific files
# available at Unidata. If you want to run this shell, you need these
# data files.

# This script gets and benchmarks against some AR4 data.

# $Id$

set -e
echo ""
#file_list="pr_A1.20C3M_8.CCSM.atmm.1870-01_cat_1999-12.nc tauu_A1.20C3M_8.CCSM.atmm.1870-01_cat_1999-12.nc thetao_O1.SRESA1B_2.CCSM.ocnm.2000-01_cat_2099-12.nc usi_O1.20C3M_8.CCSM.icem.1870-01_cat_1999-12.nc"
#file_list="pr_A1.20C3M_8.CCSM.atmm.1870-01_cat_1999-12.nc tauu_A1.20C3M_8.CCSM.atmm.1870-01_cat_1999-12.nc usi_O1.20C3M_8.CCSM.icem.1870-01_cat_1999-12.nc"
file_list="pr_A1.20C3M_8.CCSM.atmm.1870-01_cat_1999-12.nc"
echo " *** Getting sample AR4 files $file_list"

# Get the files.
for f1 in $file_list
do
    if ! test -f $f1; then
	wget ftp://ftp.unidata.ucar.edu/pub/netcdf/sample_data/ar4/$f1.gz
	gunzip $f1.gz
    fi
done

echo "SUCCESS!!!"

out='run_bm_ar4_pr_out.csv'
rm -rf $out
echo " *** Benchmarking pr_A1 file with various chunksizes (output to ${out})..."

# Create netCDF-4 versions of the file, with different chunksizes.
h=-h
s=0
pr_ar4_sample="pr_A1.20C3M_8.CCSM.atmm.1870-01_cat_1999-12.nc"

file_num=0
for d in -1
do
    for c0 in 4 8 16 32 64 
    do
	for c1 in 64
	do
	    for c2 in 128
	    do
		if test $d == -1; then
		    file_out="pr_A1_${c0}_${c1}_${c2}.nc"
		else
		    file_out="pr_A1_z${d}_${c0}_${c1}_${c2}.nc"
		fi
		out_files="$out_files $file_out"

		# If the output file does not yet exist, create it.
		if test -f $file_out; then
		    echo "found existing $file_out"
		else
		    cmd="./bm_file $h -f 3 -c 6:${d}:${s}:${c0}:${c1}:${c2} -o ${file_out} ${pr_ar4_sample}"
		    echo "cmd=$cmd"
# 	            bash ./clear_cache.sh
 		    if ! ($cmd >> $out); then
 			exit 1;
 		    fi
		fi

		# Turn off header next time around.
		h=
	    done
	done
    done
done

echo $out_files

# Do the horizonatal runs.
#bash ./clear_cache.sh
./tst_ar4 -h $pr_ar4_sample
for f1 in $out_files
do
#    bash ./clear_cache.sh
    ./tst_ar4 ${f1}
done

# Do the timeseries runs.
#bash ./clear_cache.sh
./tst_ar4 -t -h $pr_ar4_sample
for f1 in $out_files
do
#    bash ./clear_cache.sh
    ./tst_ar4 -t ${f1}
done

echo "SUCCESS!!!"
exit 0
