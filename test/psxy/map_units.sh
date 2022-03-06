#!/usr/bin/env bash
# Tests the use of map unit specified in -SE -SJ -SW
# We try unitless data with unit given via -S, same but no unit given
# Default to k] and n and k appended to input data values
gmt begin map_units
	gmt basemap -R0/25/3/33 -JM15c -B
	echo 5	5	3 | gmt plot -SE-d -Gred
	echo 10	5	3 | gmt plot -SJ-d -Gblue
	echo 15	5	300 | gmt plot -SJ-k -Ggreen
	echo 20	5	1.5 0 270 | gmt plot -SWd -Gorange
	echo 5	10	30	3	2 | gmt plot -SEd -Gred
	echo 10	10	30	3	2 | gmt plot -SJd -Gblue
	echo 15	10	30	300	200 | gmt plot -SJk -Ggreen
	echo 20	10	1.5 0 270 0.5 | gmt plot -SWd+i -Gorange
	echo 5	15	250 | gmt plot -SE-n -Gred
	echo 10	15	250 | gmt plot -SJ-n -Gblue
	echo 15	15	300 | gmt plot -SJ-k -Ggreen
	echo 20	15	125 0 270 | gmt plot -SWn -Gorange
	echo 5	20	30	250	150 | gmt plot -SEn -Gred
	echo 10	20	30	250	150 | gmt plot -SJn -Gblue
	echo 15	20	30	300	200 | gmt plot -SJk -Ggreen
	echo 20	20	200 0 270 | gmt plot -SWk -Gorange
	echo 5	25	30	300	200 | gmt plot -SE -Gred
	echo 10	25	30	300	200 | gmt plot -SJ -Gblue
	echo 15	25	30	300	200 | gmt plot -SJ -Ggreen
	echo 20	25	200	0 270 | gmt plot -SW -Gorange
	echo 5	30	30	250n	200k | gmt plot -SE -Gred
	echo 10	30	30	250n	200k | gmt plot -SJ -Gblue
	echo 15	30	30	250n	200k | gmt plot -SJ -Ggreen
	echo 20	30	125n	0	270	100k | gmt plot -SW+i -Gorange
gmt end show
