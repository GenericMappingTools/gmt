#!/usr/bin/env bash
gmt begin GMT_Defaults_1c
	gmt set MAP_FRAME_TYPE plain FORMAT_DATE_MAP "o dd" FORMAT_CLOCK_MAP hh FONT_ANNOT_PRIMARY +8p
	gmt basemap -R2001-9-11T/2001-9-13T/0.01/100 -JX2.25iT/2.25il -Bpxa6Hf1hg6h+l"x-axis label" -Bpya1g3p+l"y-axis label" -BWSne \
		-X2i -Bsxa1D -U"Dazed and Confused"+o-0.75i/-0.85i --GMT_LANGUAGE=pt \
		--FORMAT_TIME_STAMP="2011 Jun 13 22:10:02" 
	gmt text -R0/2.25/0/2.25 -Jx1i -N -F+f7p,Helvetica-Bold,blue+j << EOF 
-0.4   2.1 RM MAP_ANNOT_ORTHO
-0.4   1.9 RM MAP_FRAME_AXES
-0.9  -0.3 LB MAP_LOGO_POS
-0.7 -0.45 LB MAP_LOGO
0.0   -0.6 CB FORMAT_TIME_STAMP
2.1   -0.5 LM FONT_LABEL
2.35   0.9 LB FORMAT_DATE_MAP
2.35   0.6 LB FORMAT_CLOCK_MAP
2.35   0.3 LB GMT_LANGUAGE
2.4   -0.27 LM FONT_ANNOT_SECONDARY
EOF
	gmt plot -Sv0.06i+s+e -N -W0.5p,blue -Gblue << EOF 
-0.35 2.1 -0.05 2.1
-0.35 1.9 -0.05 1.9
-0.9 -0.3 -0.75 -0.85
-0.7 -0.47 -0.6 -0.66
0.0 -0.62 0.2 -0.75
2.05 -0.5 1.8 -0.5
2.3 0.9 0.66 -0.2
2.3 0.6 1.2 -0.1
2.3 0.3 1.8 -0.2
2.35 -0.27 2.1 -0.27
EOF
gmt end show
