#!/usr/bin/env bash
gmt begin GMT_tut_14
	gmt makecpt -H -Crainbow -T-20/60/10 > disc.cpt
	gmt makecpt -H -Crainbow -T-20/60 > cont.cpt
	gmt basemap -R0/6/0/9 -Jx1i -B0 -Xc
	gmt colorbar -Dx1i/1i+w4i/0.5i+h -Cdisc.cpt -Ba -B+tdiscrete
	gmt colorbar -Dx1i/3i+w4i/0.5i+h -Ccont.cpt -Ba -B+tcontinuous
	gmt colorbar -Dx1i/5i+w4i/0.5i+h -Cdisc.cpt -Ba -B+tdiscrete -I0.5
	gmt colorbar -Dx1i/7i+w4i/0.5i+h -Ccont.cpt -Ba -B+tcontinuous -I0.5
gmt end show
