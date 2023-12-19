#!/usr/bin/env bash
#
# Copyright (c) 1991-2023 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
# See LICENSE.TXT file for copying and redistribution conditions.
#
# GMT 3-D Christmas Ornament 2023
# Un-comment the CHRISTMAS setting in ConfigUserAdvanced.cmake and recompile GMT.
# Then run this script for a 3D cube that you assemble with scissors and tape!

SIDE=8		# The side dimension of the cube (cm)
#------------------------------------------------
SIDE2=$(gmt math -Q ${SIDE} 2 MUL =)
WP=$(gmt math -Q ${SIDE} 2 DIV =)
WQ=${SIDE}
XGREETC=$(gmt math -Q ${SIDE} 2.5 MUL =)
YGREET1=$(gmt math -Q ${SIDE} 0.6 MUL =)
YGREET2=$(gmt math -Q ${SIDE} 0.4 MUL =)
module=coast
arg1=-Gred
arg2=-Gdarkgreen

gmt begin christmas-cube pdf
# Middle Earth first quadrant
	gmt ${module} -B0g15 ${arg1} -R0/90/-45/45     -JQ045/${WQ}
# N pole 4 quadrants
	gmt ${module} -B0g15 ${arg1} -R0/90/45/90      -JJ045/${WP} -Y${SIDE}
	gmt ${module} -B0g15 ${arg2} -R90/180/45/90    -JJ135/${WP} -X${SIDE}  -p90
	gmt ${module} -B0g15 ${arg1} -R180/270/45/90   -JJ225/${WP} -Y${SIDE}  -p180
	gmt ${module} -B0g15 ${arg2} -R270/360/45/90   -JJ315/${WP} -X-${SIDE} -p-90
# Remaining 3 middle earth quadrants 
	gmt ${module} -B0g15 ${arg2} -R90/180/-45/45   -JQ135/${WQ} -X${SIDE} -Y-${SIDE2}
	gmt ${module} -B0g15 ${arg1} -R180/270/-45/45  -JQ225/${WQ} -X${SIDE}
	gmt ${module} -B0g15 ${arg2} -R270/360/-45/45  -JQ315/${WQ} -X${SIDE}
#	S pole 4 quadrants 
	gmt ${module} -B0g15 ${arg2} -R270/360/-90/-45 -JJ315/${WP} -X${SIDE}  -p-180
	gmt ${module} -B0g15 ${arg1} -R0/90/-90/-45    -JJ225/${WP} -Y-${SIDE} -p90
	gmt ${module} -B0g15 ${arg2} -R90/180/-90/-45  -JJ135/${WP} -X-${SIDE} -p0
	gmt ${module} -B0g15 ${arg1} -R180/270/-90/-45 -JJ225/${WP} -Y${SIDE}  -p-90
# Dashed cut lines for use with scissors
	gmt plot -R0/32/-8/16 -Jx1c -W0.25p,blue,- -N -X-24c -Y-8c <<- EOF
	>
	0	16
	0	17
	8	17
	8	16
	>
	24	-8
	24	-9
	32	-9
	32	-8
	>
	0	0
	-1	0
	-1	8
	0	8
	EOF
	echo "${XGREETC} ${YGREET1} Merry Christmas 2023" | gmt text -F+f24p,33+jCM -N -Gwhite
	echo "${XGREETC} ${YGREET2} From the GMT Team!"   | gmt text -F+f18p,33+jCM -N -Gwhite
gmt end show
