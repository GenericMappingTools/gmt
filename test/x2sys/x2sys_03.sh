#!/usr/bin/env bash
#
# This is original Figure 3 script from
# Wessel, P. (2010), Tools for analyzing intersecting tracks: the x2sys package,
# Computers & Geosciences, 36, 348-354.
# Here used as a test for the x2sys suite.

ps=x2sys_03.ps

R=181/185/0/3
gmt makecpt -Crainbow -T-80/80 > faa.cpt

# Grid the data
cat "${src:-.}"/bad/*.xyg | gmt blockmean -R$R -I1m | gmt surface -R$R -I1m -Gss_gridded_bad.nc -T0.25
gmt grdgradient ss_gridded_bad.nc -Ne0.75 -A65 -fg -Gss_gridded_bad_int.nc
gmt grdimage ss_gridded_bad.nc -Iss_gridded_bad_int.nc -Ei -JM5.5i -P -X1.75i -Y1.25i -Cfaa.cpt -B1 -BWSne --MAP_FRAME_WIDTH=3p --FORMAT_GEO_MAP=dddF > $ps
