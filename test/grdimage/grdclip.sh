#!/usr/bin/env bash
#
# Check clipping of grids on a global conic plot

ps=grdclip.ps

plot1 () {
gmt grdmath $1 -fg -I10 XNORM YNORM 0.2 MUL ADD = t.nc
gmt grdedit -T t.nc
gmt grdimage t.nc -nn -Ct.cpt -Q -R0/360/30/70 -J -E50 -O $2
}

plot2 () {
gmt grdmath $1 -fg -I10 XNORM YNORM 0.2 MUL ADD = t.nc
gmt grdimage t.nc -nn -Ct.cpt -Q -R0/360/30/70 -J -E50 -O $2
}

gmt makecpt -Crainbow -T-1.2/1.2/0.2 > t.cpt
gmt pscoast -R0/360/30/70 -JL180/50/40/60/6i -Gred -Dc -B30g30 -P -K > $ps
plot2 -R340/380/50/70 -K >> $ps
plot2 -R-20/20/30/50 -K >> $ps
plot1 -R-120/-80/40/60 -K >> $ps
plot1 -R160/200/40/60 >> $ps
