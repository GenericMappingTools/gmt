#!/bin/bash
#		GMT EXAMPLE 10
#		$Id$
#
# Purpose:	Make 3-D bar graph on top of perspective map
# GMT progs:	pscoast, pstext, psxyz
# Unix progs:	$AWK, rm
#
. ../functions.sh
ps=../example_10.ps
pscoast -Rd -JX8id/5id -Dc -Slightblue -Glightbrown -Wfaint -A1000 -p200/40 -K -U"Example 10 in Cookbook" > $ps
$AWK '{print $1, $2, $3}' agu2008.d \
	| pstext -R -J -O -K -p -D-0.2i/0 -F+f20p,Helvetica-Bold,blue=thinner+jRM >> $ps
psxyz agu2008.d -R-180/180/-90/90/1.01/100000 -J -JZ2.5il -So0.3ib1 -Gdarkgreen -Wthinner \
	--FONT_TITLE=30p,Times-Bold --MAP_TITLE_OFFSET=-0.7i \
	"-B60g60/30g30/a1p:Memberships::.AGU 2008 Membership Distribution:WSneZ" -O -p >> $ps
