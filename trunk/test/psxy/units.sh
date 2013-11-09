#!/bin/bash
#	$Id$
# Test that symbol units is picked up from datafile, symbol option, and GMT default, in that order
ps=units.ps
# No unit in data, PROJ_LENGTH_UNIT decides the size
echo "-2 2 1" | gmt psxy -JX6i -R-3/3/-3/3 -Ba1g1+0.5 -BWeSn -P -Sc -Ggray --PROJ_LENGTH_UNIT=inch -Xc -K > $ps
# Unit in data, whatever PROJ_LENGTH_UNIT says should be ignored
echo "2 2 1i" | gmt psxy -J -R -O -K -Sc -Ggray --PROJ_LENGTH_UNIT=point >> $ps
# No unit in data, PROJ_LENGTH_UNIT decides the size
echo "-2 -2 1" | gmt psxy -J -R -O -K -Sc -Ggray --PROJ_LENGTH_UNIT=cm >> $ps
# Unit in data, PROJ_LENGTH_UNIT should be ignored
echo "2 -2 1c" | gmt psxy -J -R -O -K -Sc -Ggray --PROJ_LENGTH_UNIT=inch >> $ps
# No unit in data, but unit specified by -S; PROJ_LENGTH_UNIT from gmt.conf should be ignored
echo "0 0 1" | gmt psxy -J -R -O -K -Sci -Gblack >> $ps
gmt psxy -R -J -O -T >> $ps
