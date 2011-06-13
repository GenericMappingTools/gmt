#!/bin/bash
# $Id: x2sys_2.sh,v 1.3 2011-06-13 05:01:42 guru Exp $
#
# This is original Figure 2 script from
# Wessel, P. (2010), Tools for analyzing intersecting tracks: the x2sys package,
# Computers & Geosciences, 36, 348–354.
# Here used as a test for the x2sys suite.

. ../functions.sh
header "x2sys: Reproduce Wessel (2010) Comp. & Geosci., Figure 2"

ps=x2sys_2.ps

R=181/185/0/3
makecpt -Crainbow -T-80/80/10 -Z > faa.cpt
psbasemap -R$R -JM5.5i -P -B1Wsne+ggray -K -X1.75i -Y5.75i --MAP_FRAME_WIDTH=3p --FORMAT_GEO_MAP=dddF > $ps
psxy -R -J -O data/*.xyg -Sc0.02i -Cfaa.cpt -K >> $ps
# Grid the data
cat data/*.xyg | blockmean -R$R -I1m | surface -R$R -I1m -Gss_gridded.nc -T0.25
grdgradient ss_gridded.nc -Ne0.75 -A65 -fg -Gss_gridded_int.nc
grdimage ss_gridded.nc -Iss_gridded_int.nc -Ei -J -O -K -Y-4.5i -Cfaa.cpt -B1WSne --MAP_FRAME_WIDTH=3p --FORMAT_GEO_MAP=dddF >> $ps
psxy -R -J -O -T >> $ps
rm -f faa.cpt ss_gridded_int.nc ss_gridded.nc

pscmp
