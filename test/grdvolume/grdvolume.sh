#!/bin/sh
#
#	$Id: grdvolume.sh,v 1.8 2008-04-01 14:42:40 remko Exp $

makegrd () {
xyz2grd -I1 -Gt.grd -Za $1 <<%
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
grdvolume t.grd $2 -Sk
echo xyz2grd $1 \; grdvolume $2:
grdvolume t.grd $2
echo xyz2grd $1 \; grdvolume $2 -L-1:
grdvolume t.grd $2 -L-1
}

testcase () {
makegrd -R0/2/0/2
makegrd "-R0/3/0/3 -F"
makegrd "-R0/2/0/2 -N0"
makegrd "-R0/3/0/3 -F -N0"
makegrd -R0/2/0/2 "-C0/0.8/0.4"
}

. ../functions.sh
header "Test grdvolume for various grid registrations"

testcase > grdvolume.log

diff grdvolume.log grdvolume.out > log

if [ $? != 0 ]; then
	touch fail
fi

rm -f t.grd .gmtcommands4 log

passfail grdvolume
