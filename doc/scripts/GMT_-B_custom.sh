#!/bin/bash
#	$Id$
#
. ./functions.sh

cat << EOF >| xannots.txt
416.0	ig	Devonian
443.7	ig	Silurian
488.3	ig	Ordovician
542	ig	Cambrian
EOF
cat << EOF >| yannots.txt
0	a
1	a
2	f
2.71828	ag	e
3	f
3.1415926	ag	@~p@~
4	f
5	f
6	f
6.2831852	ag	2@~p@~
EOF
psbasemap -R416/542/0/6.2831852 -JX-5i/2.5i -Bp25f5g25:,Ma:/cyannots.txt,WS+glightblue \
	-P -K > GMT_-B_custom.ps
psbasemap -R416/542/0/6.2831852 -JX-5i/2.5i -Bscxannots.txt/0,WS -O \
	--MAP_ANNOT_OFFSET_SECONDARY=10p --MAP_GRID_PEN_SECONDARY=2p >> GMT_-B_custom.ps
