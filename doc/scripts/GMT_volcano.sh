#!/bin/bash
#	$Id$
#
echo "0 0" | psxy -R-0.5/0.5/-0.5/0.5 -JX2i -P -Ba0.25g0.05WSne -Wthick -Skvolcano/2i -K > GMT_volcano.ps
cat <<END > bullseye.def
0	-0.7	M	-W0.5p,red
0	0.7	D
-0.7	0	M	-W0.5p,red
0.7	0	D
0	0	0.9	c	-Gp0/12
0	0	0.9	c	-W0.25p
0	0	0.7	c	-Gyellow -W0.25p
0	0	0.5	c	-Gp0/9
0	0	0.5	c	-W0.25p
0	0	0.3	c	-Gyellow -W0.25p
0	0	0.1	c	-Gwhite -W0.25p
END
echo "0 0" | psxy -R -J -N -Ba0.25g0.05wSnE -Wthick -Skbullseye/2i -O -X2.5i >> GMT_volcano.ps
