#!/usr/bin/env bash
#
# Test the C API for i/o involving grids

function gridset_check {
	testapi -I$1 -W$2 -Tg
	gmt grdmath gtesti.nc gtesto.nc SUB = tmp.nc
	N=(`gmt grd2xyz tmp.nc -ZTLa | uniq | wc -l`)
	if [ $N -ne 1 ]; then
		echo "gridset_check $1 $2 failed" >> fail
	fi
}

rm -f fail
gmt grdmath -R0/5/0/5 -I1 X Y MUL = gtesti.nc
# FYI: For grids, the stream and file descriptor cases are N/A
# 1. Read File and write GRID via...
gridset_check f f	# 1a. File
gridset_check f c	# 1d. Copy
gridset_check f r	# 1e. Reference
# 4. Read Copy and write GRID via...
gridset_check c f	# 4a. File
gridset_check c c	# 4d. Copy
gridset_check c r	# 4e. Reference
# 5. Read Reference and write GRID via...
gridset_check r f	# 5a. File
gridset_check r c	# 5d. Copy
gridset_check r r	# 5e. Reference
