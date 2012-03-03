#!/bin/bash
#
#	$Id$

. ../functions.sh
header "Test grdvolume for various grid registrations"

makegrd () {
xyz2grd -I1 -Gt.nc -Z $1 <<%
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
echo xyz2grd $1 \; grdvolume $2 -Sk:
grdvolume t.nc $2 -Sk
echo xyz2grd $1 \; grdvolume $2:
grdvolume t.nc $2
echo xyz2grd $1 \; grdvolume $2 -L-1:
grdvolume t.nc $2 -L-1
}

testcase () {
makegrd -R0/2/0/2
makegrd "-R0/3/0/3 -r"
makegrd "-R0/2/0/2 -N0"
makegrd "-R0/3/0/3 -r -N0"
makegrd -R0/2/0/2 "-C0/0.8/0.4"
}

testcase > grdvolume.log

# Note: a non-zero exit status of diff is not an error
diff grdvolume.log grdvolume.out --strip-trailing-cr > log || true

if [ $? != 0 ]; then
	touch fail
fi

rm -f t.nc log

passfail grdvolume
