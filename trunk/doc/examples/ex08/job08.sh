#!/bin/sh
#		GMT EXAMPLE 08
#
#		$Id: job08.sh,v 1.1 2001-09-14 18:55:04 pwessel Exp $
#
# Purpose:	Make a 3-D bar plot
# GMT progs:	grd2xyz, pstext, psxyz
# Unix progs:	echo, rm
#
grd2xyz guinea_bay.grd > $$
psxyz $$ -B1/1/1000:"Topography (m)"::.ETOPO5:WSneZ+ -R-0.1/5.1/-0.1/5.1/-5000/0 -JM5i -JZ6i -E200/30 -So0.0833333ub-5000 -P -U"Example 8 in Cookbook" -W0.25p -G240 -K > example_08.ps
echo '0.1 4.9 24 0 1 9 This is the surface of cube' | pstext -R -JM -JZ -Z0 -E200/30 -O >> example_08.ps
\rm -f $$ .gmtcommands
