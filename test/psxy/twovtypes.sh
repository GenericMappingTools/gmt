#!/usr/bin/env bash
ps=twovtypes.ps
# Plot two vectors of length 500 km pointing East and North at lats 0, 40 and 60
# Geovectors will plot with true length 500 km while Cartesian will have fixed length,
# here set to the equivalent of 500 km at Equator. Question is should psxy have the
# ability to take -SV with lengths in map distances instead of plot distances?
cat << EOF > gvec.txt
0	0	0	500
0	0	90	500
0	40	0	500
0	40	90	500
0	60	0	500
0	60	90	500
EOF
# Assign equivalent length in inches corresponding to 500 km at Equator
len=$(gmt math -Q 500 60 111.13 MUL DIV 6 MUL =)
cat << EOF > cvec.txt
0	0	0	${len}
0	0	90	${len}
0	40	0	${len}
0	40	90	${len}
0	60	0	${len}
0	60	90	${len}
EOF
# Plot as geovectors
gmt psxy -R-10/50/-5/65 -JM6i -P -Baf -S=0.2i+e -W0.5p -Gblack -K gvec.txt -Xc > $ps
# Plot as Cartesian vectors
gmt psxy -R -J -SV0.2i+e -W0.5p -Gblack -O -D4i/0 cvec.txt --PROJ_LENGTH_UNIT=inch >> $ps
