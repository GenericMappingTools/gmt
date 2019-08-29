REM             GMT EXAMPLE 37
REM
REM Purpose:    Illustrate 2-D FFT and coherence between gravity and bathymetry grids
REM GMT progs:  psbasemap, psxy, makecpt, grdfft, grdimage, grdinfo
REM DOS calls:  del
REM

echo GMT EXAMPLE 37
set ps=example_37.ps

REM Testing gmt grdfft coherence calculation with Karen Marks example data
REM Prefix of two .nc files

set G=@grav.V18.par.surf.1km.sq
set T=@mb.par.surf.1km.sq
gmt set FONT_TITLE 14p GMT_FFT kiss

gmt makecpt -Crainbow -T-5000/-3000 > z.cpt
gmt makecpt -Crainbow -T-50/25 > g.cpt
gmt grdinfo %T%.nc -Ib > bbox
set scl=1.4e-5
set sclkm=1.4e-2
gmt grdimage %T%.nc -I+a0+nt1 -Jx%scl%i -Cz.cpt -P -K -X1.474i -Y1i > %ps%
gmt psbasemap -R-84/75/-78/81 -Jx%sclkm%i -O -K -Ba -BWSne+t"Multibeam bathymetry" >> %ps%
gmt grdimage %G%.nc -I+a0+nt1 -Jx%scl%i -Cg.cpt -O -K -X3.25i >> %ps%
gmt psbasemap -R-84/75/-78/81 -Jx%sclkm%i -O -K -Ba -BWSne+t"Satellite gravity" >> %ps%

gmt grdfft %T%.nc %G%.nc -Ewk -N192/192+d+wtmp > cross.txt

gmt makecpt -Crainbow -T-1500/1500 > z.cpt
gmt makecpt -Crainbow -T-40/40 > g.cpt

gmt grdimage %T%_tmp.nc -I+a0+nt1 -Jx%scl%i -Cz.cpt -O -K -X-3.474i -Y3i >> %ps%
gmt psxy -R%T%_tmp.nc -J bbox -O -K -L -W0.5p,- >> %ps%
gmt psbasemap -R-100/91/-94/97 -Jx%sclkm%i -O -K -Ba -BWSne+t"Detrended and extended" >> %ps%

gmt grdimage %G%_tmp.nc -I+a0+nt1 -Jx%scl%i -Cg.cpt -O -K -X3.25i >> %ps%
gmt psxy -R%G%_tmp.nc -J bbox -O -K -L -W0.5p,- >> %ps%
gmt psbasemap -R-100/91/-94/97 -Jx%sclkm%i -O -K -Ba -BWSne+t"Detrended and extended" >> %ps%

gmt set FONT_TITLE 24p
gmt psxy -R2/160/0/1 -JX-6il/2.5i -Bxa2f3g3+u" km" -Byafg0.5+l"Coherency@+2@+" -BWsNe+t"Coherency between gravity and bathymetry" -O -K -X-3.25i -Y3.3i cross.txt -i0,15 -W0.5p >> %ps%
gmt psxy -R -J cross.txt -O -K -i0,15,16 -Sc0.075i -Gred -W0.25p -Ey >> %ps%
gmt psxy -R -J -O -T >> %ps%
REM Clean up
del cross.txt *_tmp.nc *.cpt bbox gmt.conf gmt.history
