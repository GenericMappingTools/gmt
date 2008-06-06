#!/bin/sh
#		GMT EXAMPLE 28
#		$Id: job28.sh,v 1.1 2008-06-06 00:00:23 guru Exp $
#
# Purpose:	Illustrates how to mix UTM data and UTM projection
# GMT progs:	makecpt, grdgradient, grdimage, grdinfo, pscoast
# GMT supplement: img2grd (to read Sandwell/Smith img files)
# Unix progs:	rm, grep, $AWK
#
ps=example_28.ps

psbasemap -R0/6/0/6 -JX1 -P -B1 -U"Example 28 in Cookbook" > $ps

# Clean up

rm -f *_i.nc .gmt*
