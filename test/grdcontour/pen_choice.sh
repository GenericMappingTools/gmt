#!/usr/bin/env bash
# Test the -W[a|c][<pen>][+c[l|f]] option given issue #1184

ps=pen_choice.ps

gmt makecpt -Cjet -T-2800/-100/100 > color.cpt
w="-Wa+c"
gmt grdcontour -P -K -Ccolor.cpt -JS0/90/2i -B+t$w -B0 @earth_relief_10m -GlBL/TR -R9/71.5/23.5/74r -A100 $w > $ps
w="-Wa+cf"
gmt grdcontour -K -O -Ccolor.cpt -J -X2.25i -B+t$w -B0 @earth_relief_10m -GlBL/TR -R -A100 $w >> $ps
w="-Wa+cl"
gmt grdcontour -K -O -Ccolor.cpt -J -X2.25i -B+t$w -B0 @earth_relief_10m -GlBL/TR -R -A100 $w >> $ps
w="-Wc+c"
gmt grdcontour -K -O -Ccolor.cpt -J -X-4.5i -Y3.25i -B+t$w -B0 @earth_relief_10m -GlBL/TR -R -A100 $w >> $ps
w="-Wc+cf"
gmt grdcontour -K -O -Ccolor.cpt -J -X2.25i -B+t$w -B0 @earth_relief_10m -GlBL/TR -R -A100 $w >> $ps
w="-Wc+cl"
gmt grdcontour -K -O -Ccolor.cpt -J -X2.25i -B+t$w -B0 @earth_relief_10m -GlBL/TR -R -A100 $w >> $ps
w="-W+c"
gmt grdcontour -K -O -Ccolor.cpt -J -X-4.5i -Y3.25i -B+t$w -B0 @earth_relief_10m -GlBL/TR -R -A100 $w >> $ps
w="-W+cf"
gmt grdcontour -K -O -Ccolor.cpt -J -X2.25i -B+t$w -B0 @earth_relief_10m -GlBL/TR -R -A100 $w >> $ps
w="-W+cl"
gmt grdcontour -O -Ccolor.cpt -J -X2.25i -B+t$w -B0 @earth_relief_10m -GlBL/TR -R -A100 $w >> $ps
