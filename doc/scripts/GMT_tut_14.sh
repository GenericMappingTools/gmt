#!/bin/bash
#	$Id$
#
gmt makecpt -Crainbow -T-20/60/10 > disc.cpt
gmt makecpt -Crainbow -T-20/60/10 -Z > cont.cpt
gmt psbasemap -R0/6.5/0/9 -Jx1i -P -B0 -K > GMT_tut_14.ps
gmt psscale -D3i/2i/4i/0.5ih -Cdisc.cpt -B+tdiscrete -O -K >> GMT_tut_14.ps
gmt psscale -D3i/4i/4i/0.5ih -Ccont.cpt -B+tcontinuous -O -K >> GMT_tut_14.ps
gmt psscale -D3i/6i/4i/0.5ih -Cdisc.cpt -B+tdiscrete -I0.5 -O -K >> GMT_tut_14.ps
gmt psscale -D3i/8i/4i/0.5ih -Ccont.cpt -B+tcontinuous -I0.5 -O >> GMT_tut_14.ps
