#!/bin/sh

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
~/gmt/src/grdvolume t.grd $2 -Sk
echo xyz2grd $1 \; grdvolume $2:
~/gmt/src/grdvolume t.grd $2
echo xyz2grd $1 \; grdvolume $2 -L-1:
~/gmt/src/grdvolume t.grd $2 -L-1
}

testcase () {
makegrd -R0/2/0/2
makegrd "-R0/3/0/3 -F"
makegrd "-R0/2/0/2 -N0"
makegrd "-R0/3/0/3 -F -N0"
makegrd -R0/2/0/2 "-C0/0.8/0.4 -Vl"
}

testcase > grdvolume.log

diff grdvolume.{log,out}

[[ $? == 0 ]] && echo "Test passed"

rm -f t.grd .gmtcommands4
