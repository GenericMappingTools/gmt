#!/bin/sh
#	$Id$
# Testing grdfft coherence calculation with Karen Marks example data

ps=coherence.ps

G=grav.V18.par.surf.1km.sq_grd
T=mb.par.surf.1km.sq_grd
gmtset FONT_TITLE 12p

makecpt -Crainbow -T-5000/-3000/100 -Z > z.cpt
makecpt -Crainbow -T-50/25/5 -Z > g.cpt
grdinfo $T -Ib > bbox
grdgradient $G -A0 -Nt1 -G${G}_int.nc
grdgradient $T -A0 -Nt1 -G${T}_int.nc
scl=1.4e-5
sclkm=1.4e-2
grdimage $T -I${T}_int.nc -Jx${scl}i -Cz.cpt -P -K -X1.474i > $ps
psbasemap -R-84/75/-78/81 -Jx${sclkm}i -O -K -Ba:."Multibeam bathymetry":WSne >> $ps
grdimage $G -I${G}_int.nc -Jx${scl}i -Cg.cpt -O -K -X3.25i >> $ps
psbasemap -R-84/75/-78/81 -Jx${sclkm}i -O -K -Ba:."Satellite gravity":WSne >> $ps

grdfft $T $G -Ewk -Qtmp -N192/192 > cross.txt
grdgradient tmp_${G} -A0 -Nt1 -Gtmp_${G}_int.nc
grdgradient tmp_${T} -A0 -Nt1 -Gtmp_${T}_int.nc

makecpt -Crainbow -T-1500/1500/100 -Z > z.cpt
makecpt -Crainbow -T-40/40/5 -Z > g.cpt

grdimage tmp_${T} -Itmp_${T}_int.nc -Jx${scl}i -Cz.cpt -O -K -X-3.474i -Y3i >> $ps
psxy -Rtmp_${T} -J bbox -O -K -L -W0.5p,- >> $ps
psbasemap -R-100/91/-94/97 -Jx${sclkm}i -O -K -Ba:."Detrended and extended":WSne >> $ps

grdimage tmp_${G} -Itmp_${G}_int.nc -Jx${scl}i -Cg.cpt -O -K -X3.25i >> $ps
psxy -Rtmp_${G} -J bbox -O -K -L -W0.5p,- >> $ps
psbasemap -R-100/91/-94/97 -Jx${sclkm}i -O -K -Ba:."Detrended and extended":WSne >> $ps

#gravfft -N192/192 $T -I$G/wck > coh.out

psxy -R2/160/0/1 -JX-6il/2.5i -Ba2f3g3:,km:/afg0.5:"Coherency@+2@+":WsNe -O -K -X-3.25i -Y3.5i cross.txt -i0,15 -W0.5p >> $ps
psxy -R -J cross.txt -O -K -i0,15,16 -Sc0.075i -Gred -W0.25p -Ey >> $ps
#psxy -R -J -O -K -W0.25p,blue coh.out >> $ps
psxy -R -J -O -T >> $ps
