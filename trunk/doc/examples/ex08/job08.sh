#!/bin/bash
#		GMT EXAMPLE 08
#		$Id: job08.sh,v 1.15 2011-06-09 04:12:31 guru Exp $
#
# Purpose:	Make a 3-D bar plot
# GMT progs:	grd2xyz, pstext, psxyz
# Unix progs:	echo, rm
#
. ../functions.sh
ps=../example_08.ps
grd2xyz guinea_bay.nc | psxyz -B1/1/1000:"Topography (m)"::.ETOPO5:WSneZ+ \
	-R-0.1/5.1/-0.1/5.1/-5000/0 -JM5i -JZ6i -p200/30 -So0.0833333ub-5000 -P \
	-U"Example 8 in Cookbook" -Wthinnest -Glightgreen -K > $ps
echo '0.1 4.9 This is the surface of cube' | pstext -R -J -JZ -Z0 \
	-F+f24p,Helvetica-Bold+jTL -p200/30 -O >> $ps
