#!/usr/bin/env bash
#
# Test the C API for i/o involving complex grids.  We generate
# two grids (real and imag) and read them in in different ways,
# do multiplexing and demultiplexing, and write them out again
# Output should be identical to input.  We use integer z values
# so that all formats can be tested.

function check_if_zero {
	gmt grdmath ${2}=$1 ${3}=$1 SUB = diff.nc
        N=(`gmt grd2xyz diff.nc -ZTLa | uniq | wc -l`)
        if [ $N -ne 1 ]; then
                echo "check_if_zero: $1 : $2 $3 not equal" >> fail
        fi
}

rm -f fail
# Create list of all grdformat codes
gmt grdconvert 2>&1 | awk '{if ($2 == "=") print $1}' | egrep -v 'sd|gd' > codes.lis
while read code; do
	echo "Try grid format $code"
	if [ $code = "bm" ]; then	# For bits we can only store 0s and 1s
		extra="2 MOD"
	else
		extra=""
	fi
	# Create two different grids with same region and size in this grid format
	gmt grdmath -R1/5/1/5 -I1 Y 1 SUB NX MUL X ADD $extra = in_real.grd=${code}
	gmt grdmath in_real.grd=${code} NX NY MUL ADD $extra  = in_imag.grd=${code}
	# Run the test with this codec
	testgrdio =$code
	# Compare input and output grids
	check_if_zero ${code} out_real.grd in_real.grd
	check_if_zero ${code} out_imag.grd in_imag.grd
	check_if_zero ${code} out_real_after_demux.grd in_real.grd
	check_if_zero ${code} out_imag_after_demux.grd in_imag.grd
done < codes.lis
