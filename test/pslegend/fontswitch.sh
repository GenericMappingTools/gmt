#!/usr/bin/env bash
#
# Testing gmt pslegend with font changes in -l
# See https://forum.generic-mapping-tools.org/t/font-change-in-plot-l-string-not-correctly-intpreted-in-legend-command/3989

# Generate dummy file outside the region
cat <<- END > dummy.dat
-10	-0
-8		0
END

gmt begin fontswitch
	gmt basemap -JX15c "-R0/1/0/1" -Bewsn
	gmt plot dummy.dat -Wthick      -l'@%1%m@%% (rot. feedb.)'
	gmt plot dummy.dat -Wthick,blue -l'@%1%m@%% (rot. pot. only)'
	gmt plot dummy.dat -Wthin       -l'm@-x@-'
	gmt plot dummy.dat -Wthin,-     -l'm@-y@-'
	gmt plot dummy.dat -Wthin,.     -l'm@-z@-'
gmt end show
