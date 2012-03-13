#!/bin/sh
#	$Id$
# Testing grdfilter's isotropic and rectangular filter
# on a spike, using both -D0 and -Dp for pixel widths.

. ./functions.sh
header "Test grdfilter for isotropic and rectangular filters"

filter () {
	grdfilter t.nc -Gf.nc $*
	grdmath f.nc DUP UPPER DIV = f.nc
}
image () {
	grdimage -JX2.8i -B20f10g10WSne -Ct.cpt -O -K $*
	if [ "$1" = "f.nc" ]; then
		grdinfo $1 | grep Command | awk '{print 50, 50, $6, $7}' | pstext -R -J -O -K -Dj0.1i/0.1i -F+jTR+f18p -W1p -Gwhite
	else
		echo "50 50 Spike" | pstext -R -J -O -K -Dj0.1i/0.1i -F+jTR+f18p -W1p -Gwhite
	fi
}
echo 25 25 100 | xyz2grd -R0/50/0/50 -I0.5 -N0 -Gt.nc
makecpt -Crainbow -T0/1/0.1 -Z > t.cpt
psscale -D3.25i/-0.35i/6i/0.1ih -P -K -Ct.cpt -Y1.2i > $ps
image t.nc >> $ps
filter -D0 -Fg30
image f.nc -X3.5i >> $ps
filter -D0 -Fb20/10
image f.nc -X-3.5i -Y3.2i >> $ps
filter -D0 -Fg15/30
image f.nc -X3.5i >> $ps
filter -Dp -Fb31/31
image f.nc -X-3.5i -Y3.2i >> $ps
filter -Dp -Fc61/61
image f.nc -X3.5i  >> $ps
psxy -R -J -O -T >> $ps

pscmp
