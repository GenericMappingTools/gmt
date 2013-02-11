#!/bin/sh
#	$Id$
#
# Compare gravfft and grdfft calculations of admittance and coherence between bathymetry and geoid over the Azores
ps=cross_spec.ps

gravfft bathy_1m.nc -Igeoid_1m.nc/wck -fg > coh.txt
gravfft bathy_1m.nc -Igeoid_1m.nc/wk  -fg > adm.txt
grdfft  bathy_1m.nc  geoid_1m.nc -Ewk -fg > cross.txt

# coh: gravfft in red, grdfft in green
psxy coh.txt -R3/1000/0/1 -JX-6il/4.5i -P -Ba1g3:"Wavelength (km)":/afg:"Coherence":WSne -Sc0.05i -Gred -Ey0.2c/0.5p,red -K -X1.25i > $ps
psxy cross.txt -R -J -O -K -Sp -Ey0.1c/0.5p,green -i0,15,16 >> $ps
psxy cross.txt -R -J -O -K -W0.25p,green -i0,15,16 >> $ps
# adm: gravfft in red, grdfft in green
psxy adm.txt -R3/1000/0/2 -JX-6il/4.5i -O -Ba1g3:"Wavelength (km)":/afg:"Admittance (mGal/km)":WSne -Sc0.05i -Ey0.2c/0.5p,red -Gred -K -i0,1s1000,2s1000 -Y5.3i >> $ps
psxy cross.txt -R -J -O -K -Sp -Ey0.1c/0.5p,green -i0,11s1000,12s1000 >> $ps
psxy cross.txt -R -J -O -W0.25p,green -i0,11s1000,12s1000 >> $ps
