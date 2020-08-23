#!/usr/bin/env bash
# Illustrate the problem with grdsample -T
gmt begin GMT_grid2pix ps
	gmt subplot begin 2x1 -F6i/2.5i -M3p -A+jTR+o-0.2i/0
		gmt subplot set 0
		gmt math -T0/7/0.02 T PI MUL COS = | gmt plot -R0/6.7/-1.5/1.7 -W2p -Bx1 -By0g10 -BWS --MAP_FRAME_TYPE=graph --MAP_GRID_PEN_PRIMARY=0.25p,-
		gmt basemap -Bx0g1+0.5 -By0 -BS
		gmt math -T0/7/0.02 T PI MUL 4 DIV 2.25 SUB COS = | gmt plot -W0.25p
		gmt math -T0/6/1 T PI MUL COS = | gmt plot -Sc0.1i -Gred -W0.25p -N
		gmt math -T0.5/6.5/1 T 0 MUL = | gmt plot -St0.1i -Gred -W0.25p -N
		gmt math -T0/6/1 T PI MUL 4 DIV 2.25 SUB COS = | gmt plot -Sc0.1i -Gblue -W0.25p -N
		gmt math -T0.5/6.5/1 T PI MUL 4 DIV 2.25 SUB COS = | gmt plot -St0.1i -Gblue -W0.25p -N
		gmt text -N -F+f+j <<- EOF
		6.9 -1.7 18p,Times-Italic RT x
		-0.2 1.6 18p,Times-Italic RT z
		-0.2 1.0 16p,Times-Italic RM a@-n/2@-
		-0.2 0.0 14p,Helvetica RM 0
		EOF
		gmt subplot set 1
		gmt set PS_SCALE_X 0.6 PS_SCALE_Y 0.6 FONT_ANNOT_PRIMARY 14p FONT_LABEL 16p MAP_LABEL_OFFSET 0
		gmt math -T0/0.5/0.001 T PI MUL COS = | gmt plot -R0/0.5/0/1.2 -W2p -Bx1f+l"Wavenumber, @%6%k@%%" -By1f+l"|@%6%T(k@-j@-)@%%|" -BWS --MAP_FRAME_TYPE=graph --MAP_GRID_PEN_PRIMARY=0.25p,-
		gmt text -N -F+f+j <<- EOF
		0.5 -0.1 14p,Times-Italic TC k@-n/2@-
		EOF
	gmt subplot end
gmt end show
