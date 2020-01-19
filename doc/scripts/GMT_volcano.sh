#!/usr/bin/env bash
gmt begin GMT_volcano
echo "0 0" | gmt plot -R-0.5/0.5/-0.5/0.5 -JX2i -Ba0.25g0.05 -BWSne -Wthick -Skvolcano/2i
cat <<END > bullseye.def
0	-0.7	M	-W0.5p,red
0	0.7	D
-0.7	0	M	-W0.5p,red
0.7	0	D
0	0	0.9	c	-Gp12
0	0	0.9	c	-W0.25p
0	0	0.7	c	-Gyellow -W0.25p
0	0	0.5	c	-Gp9
0	0	0.5	c	-W0.25p
0	0	0.3	c	-Gyellow -W0.25p
0	0	0.1	c	-Gwhite -W0.25p
END
echo "0 0" | gmt plot -N -Ba0.25g0.05 -BwSnE -Wthick -Skbullseye/2i -X2.5i
gmt end show
