#!/bin/bash
# $Id$
#
# This is original Figure 1 script from
# Wessel, P. (2010), Tools for analyzing intersecting tracks: the x2sys package,
# Computers & Geosciences, 36, 348–354.
# Here used as a test for the x2sys suite.

. functions.sh
header "Reproduce Wessel (2010) Comp. & Geosci., Figure 1"

ps=x2sys_1.ps

R=181/185/0/3
Re=181/185/0/3.01527387994
D=$src/grav.16.1.B.img
makecpt -Crainbow -T-80/80/10 -Z > faa.cpt
grdgradient -Ne0.75 -A65 -fg $src/ss_faa.nc -Gss_faa_int.nc

grdimage $src/ss_faa.nc -Iss_faa_int.nc -JM5.5i -P -K -Cfaa.cpt -X1.75i -Y2.5i > $ps
psxy -R$R -J $src/data/*.xyg -W0.25p -O -K -B1WSne --MAP_FRAME_WIDTH=3p --FORMAT_GEO_MAP=dddF >> $ps
psscale -Cfaa.cpt -D2.5i/-0.5i/4.5i/0.15ih -O -E -B20f10/:"mGal": >> $ps

pscmp
