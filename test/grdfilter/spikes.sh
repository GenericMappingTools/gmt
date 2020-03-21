#!/usr/bin/env bash
# Testing gmt grdfilter's isotropic and rectangular filter
# on a spike, using both -D0 and -Dp for pixel widths.

ps=spikes.ps

if [[ ${HAVE_GLIB_GTHREAD} =~ TRUE|ON ]]; then
  _thread_opt=-x+a
fi

filter () {
	gmt grdfilter t.nc -Gf.nc ${_thread_opt} $*
	gmt grdmath f.nc DUP UPPER DIV = f.nc
}
image () {	# First 3 args for gmt grdimage; $4 is text for gmt pstext
	gmt grdimage -JX2.8i -B20f10g10 -BWSne -Ct.cpt -O -K $1 $2 $3
	echo "50 50 $4" | gmt pstext -R -J -O -K -Dj0.1i/0.1i -F+jTR+f18p -W1p -Gwhite
}
echo 25 25 100 | gmt xyz2grd -R0/50/0/50 -I0.5 -di0 -Gt.nc
gmt makecpt -Crainbow -T0/1 > t.cpt
gmt psscale -Dx3.25i/-0.35i+w6i/0.1i+h+jTC -P -K -Ba0.1 -Ct.cpt -Y1.2i > $ps
image t.nc -X0i -Y0i "Spike" >> $ps
filter -D0 -Fg30
image f.nc -X3.5i -Y0i "-D0 -Fg30" >> $ps
filter -D0 -Fb20/10
image f.nc -X-3.5i -Y3.2i "-D0 -Fb20/10" >> $ps
filter -D0 -Fg15/30
image f.nc -X3.5i -Y0i "-D0 -Fg15/30" >> $ps
filter -Dp -Fb31/31
image f.nc -X-3.5i -Y3.2i "-Dp -Fb31/31" >> $ps
filter -Dp -Fc61/61
image f.nc -X3.5i -Y0i "-Dp -Fc61/61" >> $ps
gmt psxy -R -J -O -T >> $ps

