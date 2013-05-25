#!/bin/sh -xv
#	$Id: coherence.sh 11495 2013-05-16 19:40:41Z pwessel $
# Testing gmt grdfft coherence calculation with Karen Marks example data

ps=coherence.ps

# Prefix of two .nc files
G=grav.V18.par.surf.1km.sq
T=mb.par.surf.1km.sq
gmt gmtset FONT_TITLE 12p

gmt makecpt -Crainbow -T-5000/-3000/100 -Z > z.cpt
gmt makecpt -Crainbow -T-50/25/5 -Z > g.cpt
gmt grdinfo $T.nc -Ib > bbox
gmt grdgradient $G.nc -A0 -Nt1 -G${G}_int.nc
gmt grdgradient $T.nc -A0 -Nt1 -G${T}_int.nc
scl=1.4e-5
sclkm=1.4e-2
gmt grdimage $T.nc -I${T}_int.nc -Jx${scl}i -Cz.cpt -P -K -X1.474i > $ps
gmt psbasemap -R-84/75/-78/81 -Jx${sclkm}i -O -K -Ba -BWSne+t"Multibeam bathymetry" >> $ps
gmt grdimage $G.nc -I${G}_int.nc -Jx${scl}i -Cg.cpt -O -K -X3.25i >> $ps
gmt psbasemap -R-84/75/-78/81 -Jx${sclkm}i -O -K -Ba -BWSne+t"Satellite gravity" >> $ps

gmt grdfft $T.nc $G.nc -Ewk -N192/192+d+wtmp > cross.txt
gmt grdgradient ${G}_tmp.nc -A0 -Nt1 -G${G}_tmp_int.nc
gmt grdgradient ${T}_tmp.nc -A0 -Nt1 -G${T}_tmp_int.nc

gmt makecpt -Crainbow -T-1500/1500/100 -Z > z.cpt
gmt makecpt -Crainbow -T-40/40/5 -Z > g.cpt

gmt grdimage ${T}_tmp.nc -I${T}_tmp_int.nc -Jx${scl}i -Cz.cpt -O -K -X-3.474i -Y3i >> $ps
gmt psxy -R${T}_tmp.nc -J bbox -O -K -L -W0.5p,- >> $ps
gmt psbasemap -R-100/91/-94/97 -Jx${sclkm}i -O -K -Ba -BWSne+t"Detrended and extended" >> $ps

gmt grdimage ${G}_tmp.nc -I${G}_tmp_int.nc -Jx${scl}i -Cg.cpt -O -K -X3.25i >> $ps
gmt psxy -R${G}_tmp.nc -J bbox -O -K -L -W0.5p,- >> $ps
gmt psbasemap -R-100/91/-94/97 -Jx${sclkm}i -O -K -Ba -BWSne+t"Detrended and extended" >> $ps

gmt psxy -R2/160/0/1 -JX-6il/2.5i -Bxa2f3g3+u" km" -Byafg0.5+l"Coherency@+2@+" -BWsNe -O -K -X-3.25i -Y3.5i cross.txt -i0,15 -W0.5p >> $ps
gmt psxy -R -J cross.txt -O -K -i0,15,16 -Sc0.075i -Gred -W0.25p -Ey >> $ps
gmt psxy -R -J -O -T >> $ps
