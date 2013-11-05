#!/bin/bash
#
#	$Id$

ps=grdtime.ps
# tsu_chile60.nc contains predicted tsunami travel times in hours since the earthquake
# Zero hour is 1960-05-22T19:11:17.0.  Make a map showing UTC hours as contour annotations.

gmt grdcontour tsu_chile60.nc -JM6i -P -A1+f9p -C0.5 -L0/4.75 -GlZ-/LT -K -Wc0.25p,- -Xc -fg,T \
	--TIME_EPOCH=1960-05-22T19:11:17.0 --TIME_UNIT=h \
	--FORMAT_DATE_MAP= --FORMAT_CLOCK_MAP=hh:mm > $ps
gmt grdcontour tsu_chile60.nc -J -A1+f9p -C0.5 -L5.25/50 -GlZ-/LT -O -K -Wc0.25p,- -fg,T \
	--TIME_EPOCH=1960-05-22T19:11:17.0 --TIME_UNIT=h \
	--FORMAT_DATE_MAP= --FORMAT_CLOCK_MAP=hh:mm >> $ps
gmt grdcontour tsu_chile60.nc -J -A1+f10p+v -L4.75/5.25 -GlZ-/LT -O -K -fg,T \
	--TIME_EPOCH=1960-05-22T19:11:17.0 --TIME_UNIT=hh:mm \
	--FORMAT_DATE_MAP=yyyy/mm/dd --FORMAT_CLOCK_MAP=hh:mm >> $ps
echo 74:30W 39:30S | gmt psxy -Rtsu_chile60.nc -J -O -K -Sa0.1i -Gblack >> $ps
gmt pscoast -R -J -O -Ggray -Baf --FORMAT_GEO_MAP=dddF >> $ps
