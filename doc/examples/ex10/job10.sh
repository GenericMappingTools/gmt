#!/bin/sh
#		GMT EXAMPLE 10
#		$Id: job10.sh,v 1.9 2008-02-22 21:10:42 remko Exp $
#
# Purpose:	Make 3-D bar graph on top of perspective map
# GMT progs:	pscoast, pstext, psxyz
# Unix progs:	$AWK, rm
#
ps=example_10.ps
pscoast -Rd -JX8id/5id -Dc -Gblack -E200/40 -K -U"Example 10 in Cookbook" > $ps
psxyz agu.d -R-180/180/-90/90/1/100000 -J -JZ2.5il -So0.3ib1 -Ggray -Wthinner \
	-B60g60/30g30/a1p:Memberships:WSneZ -O -K -E200/40 >> $ps
$AWK '{print $1-10, $2, 20, 0, 0, "RM", $3}' agu.d \
	| pstext -Rd -J -O -K -E200/40 -Gwhite -Sthinner >> $ps
echo "4.5 6 30 0 5 BC AGU 1991 Membership Distribution" | pstext -R0/11/0/8.5 -Jx1i -O >> $ps
rm -f .gmt*
