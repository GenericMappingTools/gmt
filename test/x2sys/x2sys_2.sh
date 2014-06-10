#!/bin/bash
# $Id$
#
# This is original Figure 2 script from
# Wessel, P. (2010), Tools for analyzing intersecting tracks: the x2sys package,
# Computers & Geosciences, 36, 348-354.
# Here used as a test for the x2sys suite.

ps=x2sys_2.ps

R=181/185/0/3
gmt makecpt -Crainbow -T-80/80/10 -Z > faa.cpt
gmt psbasemap -R$R -JM5.5i -P -B1 -BWsne+ggray -K -X1.75i -Y5.75i --MAP_FRAME_WIDTH=3p --FORMAT_GEO_MAP=dddF > $ps
gmt psxy -R -J -O "${src:-.}"/data/*.xyg -Sc0.02i -Cfaa.cpt -K >> $ps
# Grid the data
cat "${src:-.}"/data/*.xyg | gmt blockmean -R$R -I1m | gmt surface -R$R -I1m -Gss_gridded.nc -T0.25
gmt grdgradient ss_gridded.nc -Ne0.75 -A65 -fg -Gss_gridded_int.nc
gmt grdimage ss_gridded.nc -Iss_gridded_int.nc -Ei -J -O -K -Y-4.5i -Cfaa.cpt -B1 -BWSne --MAP_FRAME_WIDTH=3p --FORMAT_GEO_MAP=dddF >> $ps
gmt psxy -R -J -O -T >> $ps

