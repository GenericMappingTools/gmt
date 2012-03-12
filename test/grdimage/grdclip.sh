#!/bin/bash
#	$Id$
#
# Check clipping of grids on a global conic plot

. functions.sh
header "Test grdimage for clipping grids"

plot1 () {
grdmath $1 -fg -I10 Xn Yn 0.2 MUL ADD = t.nc
grdedit -T t.nc
grdimage t.nc -nn -Ct.cpt -Q -R0/360/30/70 -J -E50 -O $2
}

plot2 () {
grdmath $1 -fg -I10 Xn Yn 0.2 MUL ADD = t.nc
grdimage t.nc -nn -Ct.cpt -Q -R0/360/30/70 -J -E50 -O $2
}

makecpt -Crainbow -T-1.2/1.2/0.2 > t.cpt
pscoast -R0/360/30/70 -JL180/50/40/60/6i -Gred -Dc -B30g30 -P -K > $ps
plot2 -R340/380/50/70 -K >> $ps
plot2 -R-20/20/30/50 -K >> $ps
plot1 -R-120/-80/40/60 -K >> $ps
plot1 -R160/200/40/60 >> $ps

pscmp
