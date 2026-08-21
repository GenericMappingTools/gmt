#!/usr/bin/env bash
# Tests the use of map unit specified in -SE -SJ -SW
# We try unitless data with unit given via -S, same but no unit given
# Default to k] and n and k appended to input data values
gmt begin map_units
	gmt basemap -R0/25/3/33 -JM15c -B
	echo 5	5	0	3 | gmt plot3d -SE-d -Gred
	echo 10	5	0	3 | gmt plot3d -SJ-d -Gblue
	echo 15	5	0	300 | gmt plot3d -SJ-k -Ggreen
	echo 20	5	0	1.5 0 270 | gmt plot3d -SWd -Gorange
	echo 5	10	0	30	3	2 | gmt plot3d -SEd -Gred
	echo 10	10	0	30	3	2 | gmt plot3d -SJd -Gblue
	echo 15	10	0	30	300	200 | gmt plot3d -SJk -Ggreen
	echo 20	10	0	1.5 0 270 0.5 | gmt plot3d -SWd+i -Gorange
	echo 5	15	0	250 | gmt plot3d -SE-n -Gred
	echo 10	15	0	250 | gmt plot3d -SJ-n -Gblue
	echo 15	15	0	300 | gmt plot3d -SJ-k -Ggreen
	echo 20	15	0	125 0 270 | gmt plot3d -SWn -Gorange
	echo 5	20	0	30	250	150 | gmt plot3d -SEn -Gred
	echo 10	20	0	30	250	150 | gmt plot3d -SJn -Gblue
	echo 15	20	0	30	300	200 | gmt plot3d -SJk -Ggreen
	echo 20	20	0	200 0 270 | gmt plot3d -SWk -Gorange
	echo 5	25	0	30	300	200 | gmt plot3d -SE -Gred
	echo 10	25	0	30	300	200 | gmt plot3d -SJ -Gblue
	echo 15	25	0	30	300	200 | gmt plot3d -SJ -Ggreen
	echo 20	25	0	200	0 270 | gmt plot3d -SW -Gorange
	echo 5	30	0	30	250n	200k | gmt plot3d -SE -Gred
	echo 10	30	0	30	250n	200k | gmt plot3d -SJ -Gblue
	echo 15	30	0	30	250n	200k | gmt plot3d -SJ -Ggreen
	echo 20	30	0	125n	0	270	100k | gmt plot3d -SW+i -Gorange
gmt end show
