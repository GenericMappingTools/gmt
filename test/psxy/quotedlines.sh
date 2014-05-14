#!/bin/bash
#	$Id$
# Place 3 lines with text along them
ps=quotedlines.ps
echo "> -L\"The first curve\"" > data.txt
gmtmath -T0/180/1 T SIND = >> data.txt
echo "> -L\"The second curve\"" >> data.txt
gmtmath -T0/180/1 T SIND 2 ADD = >> data.txt
echo "> -L\"The third curve\"" >> data.txt
gmtmath -T0/180/1 T SIND 4 ADD = >> data.txt
psxy -R-5/185/-1/6 -JX6i/9i -P -B0 -W1p,red -Sqn2:+v+f12p+Lh+p0.25p,blue data.txt --PS_COMMENTS=true > $ps
