#!/bin/sh
#	$Id$
#
# Compute admittance for synthetic data created by grdfft etc.
ps=admittance.ps

# 4 panels of topo and grav, with top profile of admittance & coherence
# NOT FINISHED
# 1. Create a bathymetry data set with 3 seamounts
grdmath -R0/512/0/512 -I4 -r \
	310 290 CDIST 40 DIV 2 POW 4.5 MUL NEG EXP 3000 MUL \
	290 205 CDIST 50 DIV 2 POW 4.5 MUL NEG EXP 4000 MUL ADD \
	210 280 CDIST 60 DIV 2 POW 4.5 MUL NEG EXP 2500 MUL ADD = z.nc
grdedit z.nc -R0/512000/0/512000	# Change to km
makecpt -Crainbow -T-5000/0/500 -Z > z.cpt
makecpt -Crainbow -T-5000/0/500 -Z > w.cpt
grdimage z.nc -Cz.cpt -JX3i -P -BaWSne > $ps
exit
makecpt -Crainbow -T-100/100/10 -Z > g.cpt
grdimage z.nc -Cz.cpt -J3i -P -BaWSne -K > $ps
# 2. COmpute flexure
gravfft z.nc -T12000/2600/3300/1000 -Q -Z-12000 -N1024/1024+w -L -Gmoho_flex.nc
# 3. Gravity from flexure
gravfft moho_flex.nc -T30000/2600/3300/1000+ -Ff -E4 -Gfaa_flex.nc
grdimage z.nc -Cz.cpt -J3i -P -BaWSne -K > $ps
# 4. Gavity from flexure and seamounts

gravfft z.nc -Ff -E4 -Gfaa_load.nc
gravfft bathy_1m.nc geoid_1m.nc -Iwk  -fg > adm.txt
grdfft  bathy_1m.nc geoid_1m.nc -Ewk  -fg > cross.txt

# coh: gravfft in red, grdfft in green
psxy coh.txt -R3/1000/0/1 -JX-6il/4.5i -P -Ba1g3:"Wavelength (km)":/afg:"Coherence":WSne -Sc0.05i -Gred -Ey0.2c/0.5p,red -K -X1.25i > $ps
psxy cross.txt -R -J -O -K -Sp -Ey0.1c/0.5p,green -i0,15,16 >> $ps
psxy cross.txt -R -J -O -K -W0.25p,green -i0,15,16 >> $ps
# adm: gravfft in red, grdfft in green
psxy adm.txt -R3/1000/0/2 -JX-6il/4.5i -O -Ba1g3:"Wavelength (km)":/afg:"Admittance (mGal/km)":WSne -Sc0.05i -Ey0.2c/0.5p,red -Gred -K -i0,1s1000,2s1000 -Y5.3i >> $ps
psxy cross.txt -R -J -O -K -Sp -Ey0.1c/0.5p,green -i0,11s1000,12s1000 >> $ps
psxy cross.txt -R -J -O -W0.25p,green -i0,11s1000,12s1000 >> $ps
