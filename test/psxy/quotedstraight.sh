#!/bin/bash
#	$Id$
# Place 3 lines with text along the straight lines
ps=quotedstraight.ps
echo "> -L\"The first curve\"" > data.txt
gmtmath -T0/180/45 T 180 DIV = >> data.txt
echo "> -L\"The second curve\"" >> data.txt
gmtmath -T0/180/45 T 180 DIV 2 ADD = >> data.txt
echo "> -L\"The third curve\"" >> data.txt
gmtmath -T0/180/45 T 180 DIV 4 ADD = >> data.txt
#psxy -R-5/185/-1/6 -JX6i/9i -P -B0 -W1p,red -Sqn2:+f12p+Lh+p0.25p,blue+ggray data.txt --PS_COMMENTS=true > $ps
#psxy -R-5/185/-1/6 -JX6i/9i -P -K -W1p,red -Sqn2:+f12p+Lh+p0.25p+o+e data.txt --PS_COMMENTS=true > $ps
psxy -R-5/185/-1/6 -JX6i/9i -P -K -W1p,red -Sqn2:+f12p+Lh+o+e data.txt --PS_COMMENTS=true > $ps
psbasemap -R -J -O -K -B0 -B+gyellow --PS_COMMENTS=true >> $ps
psclip -R -J -O -Ca >> $ps
