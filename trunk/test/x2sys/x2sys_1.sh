#!/bin/bash
# $Id: x2sys_1.sh,v 1.1 2011-06-13 04:07:26 guru Exp $
#
# This is original Figure 1 script from
# Wessel, P. (2010), Tools for analyzing intersecting tracks: the x2sys package,
# Computers & Geosciences, 36, 348–354.
# Here used as a test for the x2sys suite.

. ../functions.sh
header "Test x2sys to reproduce Wessel (2010) Comp. & Geosci., Figure 1"

ps=Wessel_x2sys_Fig_1.ps

R=181/185/0/3
Re=181/185/0/3.01527387994
D=grav.16.1.B.img
makecpt -Crainbow -T-80/80/10 -Z > faa.cpt
img2grd $D -R$R -T1 -E -Gss_faa.nc -S
grdgradient -Ne0.75 -A65 -fg ss_faa.nc -Gss_faa_int.nc

grdimage ss_faa.nc -Iss_faa_int.nc -JM5.5i -P -K -Cfaa.cpt -X1.75i -Y2.5i > $ps
psxy -R$R -J data/*.xyg -W0.25p -O -K -B1WSne --MAP_FRAME_WIDTH=3p --FORMAT_GEO_MAP=dddF >> $ps
psscale -Cfaa.cpt -D2.5i/-0.5i/4.5i/0.15ih -O -E -B20f10/:"mGal": >> $ps
rm -f faa.cpt ss_faa_int.nc ss_faa.nc

pscmp
