#!/bin/bash
#
#	$Id$

makegrd () {
gmt xyz2grd -I1 -Gt.nc -Z $1 <<%
1
1
1
1
1
0
1
0
0
%
echo
echo gmt xyz2grd $1 \; gmt grdvolume $2 -Sk:
gmt grdvolume t.nc $2 -Sk
echo gmt xyz2grd $1 \; gmt grdvolume $2:
gmt grdvolume t.nc $2
echo gmt xyz2grd $1 \; gmt grdvolume $2 -L-1:
gmt grdvolume t.nc $2 -L-1
}

testcase () {
makegrd -R0/2/0/2
makegrd "-R0/3/0/3 -r"
makegrd "-R0/2/0/2 -N0"
makegrd "-R0/3/0/3 -r -N0"
makegrd -R0/2/0/2 "-C0/0.8/0.4"
}

testcase > gmt grdvolume.log

# Note: a non-zero exit status of diff is not an error
diff gmt grdvolume.log gmt grdvolume.out --strip-trailing-cr > log || true
