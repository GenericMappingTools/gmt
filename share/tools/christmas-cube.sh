#!/usr/bin/env bash
#
# Copyright (c) 1991-2023 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
# See LICENSE.TXT file for copying and redistribution conditions.
#
# GMT 3-D Christmas Ornament 2023!
# Un-comment the CHRISTMAS setting in ConfigUserAdvanced.cmake and recompile GMT.
# Then run this script for a 3D cube that you assemble with scissors and tape!

SIDE=8		# The side dimension of the cube (cm)
#------------------------------------------------
WQ=${SIDE}
WP=$(gmt math -Q ${SIDE} 2 DIV =)
H=$(gmt math -Q ${SIDE} 2 SQRT DIV =)
XSHIFT=$(gmt math -Q ${SIDE} 2 MUL NEG =)
YSHIFT=$(gmt math -Q ${SIDE} 0 ADD =)

gmt begin christmas-cube pdf
# N pole area and middle Earth
	gmt coast -R0/90/-45/45     -JQ45/${WQ}   -B0g15 -Gred
	gmt coast -R0/90/45/90      -JJ45/${WP}   -B0g15 -Gred -Y${WQ}
	echo 45 90 | gmt plot -Sa16p -W0.5p -Gyellow
	gmt coast -R90/180/-45/45   -JQ135/${WQ}  -B0g15 -Gdarkgreen -X${SIDE} -Y-${WQ}
	gmt coast -R90/180/45/90    -JJ135/${WP}  -B0g15 -Gdarkgreen -Y${WQ}
	echo 135 90 | gmt plot -Sa16p -W0.5p -Gyellow
	gmt coast -R180/270/-45/45  -JQ225/${WQ}  -B0g15 -Gred -X${SIDE} -Y-${WQ}
	echo "225 8 Merry Christmas 2023" | gmt text -F+f24p,33+jCM -N -Gwhite
	echo "225 -7 From the GMT Team!"   | gmt text -F+f18p,33+jCM -N -Gwhite
	gmt coast -R180/270/45/90   -JJ225/${WP}  -B0g15 -Gred -Y${WQ}
	echo 225 90 | gmt plot -Sa16p -W0.5p -Gyellow
	gmt coast -R270/360/-45/45  -JQ315/${WQ}  -B0g15 -Gdarkgreen -X${SIDE} -Y-${WQ}
	gmt coast -R270/360/45/90   -JJ315/${WP}  -B0g15 -Gdarkgreen -Y${WQ}
	echo 315 90 | gmt plot -Sa16p -W0.5p -Gyellow
# S pole area 
	gmt coast -R0/90/-90/-45    -JJ45/${WP}   -B0g15 -Gred -X${XSHIFT} -Y-${YSHIFT} -p180
	echo 45 -90 | gmt plot -Sa16p -W0.5p -Gyellow -p
	gmt coast -R90/180/-90/-45  -JJ135/${WP}  -B0g15 -Gdarkgreen -X${SIDE} -p
	echo 135 -90 | gmt plot -Sa16p -W0.5p -Gyellow -p
	gmt coast -R180/270/-90/-45 -JJ225/${WP}  -B0g15 -Gred -X${SIDE} -p
	echo 225 -90 | gmt plot -Sa16p -W0.5p -Gyellow -p
	gmt coast -R270/360/-90/-45 -JJ3155/${WP} -B0g15 -Gdarkgreen -X${SIDE} -p
	echo 315 -90 | gmt plot -Sa16p -W0.5p -Gyellow -p
# Dashed cut lines for use with scissors
	gmt plot -R0/32/-4/12 -Jx1c -W0.25p,blue,- -N -X-32c -Y-4c <<- EOF
	>
	32	0
	33	0
	33	8
	32	8
	32.707	8.707
	28	13.3
	24	9
	24	8
	>
	16	8
	16	9
	12	13.3
	8	9.3
	8	8
	>
	28	13
	28	12
	>
	12	13
	12	12
	>
	0	0
	-1	0
	-1	8
	0	8
	>
	0	0
	-0.707	-0.707
	4	-5
	8	-1
	8	0
	>
	16	0
	16	-1
	20	-5
	24	-1
	24	0
	>
	4	-5
	4	-4
	>
	20	-5
	20	-4
	EOF
gmt end show
