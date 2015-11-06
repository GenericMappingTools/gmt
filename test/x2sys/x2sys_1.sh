#!/bin/bash
# $Id$
#
# This is original Figure 1 script from
# Wessel, P. (2010), Tools for analyzing intersecting tracks: the x2sys package,
# Computers & Geosciences, 36, 348-354.
# Here used as a test for the x2sys suite.

ps=x2sys_1.ps

R=181/185/0/3
gmt makecpt -Crainbow -T-80/80/10 -Z > faa.cpt
gmt grdgradient -Ne0.75 -A65 -fg ss_faa.nc -Gss_faa_int.nc

gmt grdimage ss_faa.nc -Iss_faa_int.nc -JM5.5i -P -K -Cfaa.cpt -X1.75i -Y2.5i > $ps
gmt psxy -R$R -J "${src:-.}"/data/*.xyg -W0.25p -O -K -B1 -BWSne --MAP_FRAME_WIDTH=3p --FORMAT_GEO_MAP=dddF >> $ps
gmt psscale -Cfaa.cpt -D2.5i/-0.5i+w4.5i/0.15i+h+jTC -O -E -Bx20f10 -By+l"mGal" >> $ps

