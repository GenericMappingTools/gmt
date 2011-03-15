#!/bin/bash
#		GMT EXAMPLE 10
#		$Id: job10.sh,v 1.13 2011-03-15 02:06:31 guru Exp $
#
# Purpose:	Make 3-D bar graph on top of perspective map
# GMT progs:	pscoast, pstext, psxyz
# Unix progs:	$AWK, rm
#
. ../functions.sh
ps=../example_10.ps
pscoast -Rd -JX8id/5id -Dc -Gblack -p200/40 -K -U"Example 10 in Cookbook" > $ps
psxyz agu2008.d -R-180/180/-90/90/1/100000 -J -JZ2.5il -So0.3ib1 -Ggray -Wthinner \
	-B60g60/30g30/a1p:Memberships:WSneZ -O -K -p200/40 >> $ps
$AWK '{print $1, $2, $3}' agu2008.d \
	| pstext -Rd -J -O -K -p200/40 -D-0.2i/0 -F+f20p,Helvetica-Bold,white=thinner+jRM >> $ps
echo "4.5 6 AGU 2008 Membership Distribution" | pstext -R0/11/0/8.5 -Jx1i \
	-F+f30p,Times-Bold+jBC -O >> $ps
