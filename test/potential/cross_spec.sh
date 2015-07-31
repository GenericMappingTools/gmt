#!/bin/bash
#	$Id$
#
# Compare gmt gravfft and gmt grdfft calculations of admittance and coherence between bathymetry and geoid over the Azores
ps=cross_spec.ps

gmt gravfft bathy_1m.nc geoid_1m.nc -N+d -Iwck -fg > coh.txt
gmt gravfft bathy_1m.nc geoid_1m.nc -N+d -Iwk  -fg > adm.txt
gmt grdfft  bathy_1m.nc geoid_1m.nc -N+d -Ewk  -fg > cross.txt

# coh: gmt gravfft in red, gmt grdfft in green
gmt psxy coh.txt -R3/1000/0/1 -JX-6il/4.5i -P -Bxa1g3+l"Wavelength (km)" -Byafg+l"Coherence" -BWSne -Sc0.05i -Gred -Ey0.2c/0.5p,red -K -X1.25i > $ps
gmt psxy cross.txt -R -J -O -K -Sp -Ey0.1c/0.5p,green -i0,15,16 >> $ps
gmt psxy cross.txt -R -J -O -K -W0.25p,green -i0,15 >> $ps
# adm: gmt gravfft in red, gmt grdfft in green
gmt psxy adm.txt -R3/1000/0/2 -JX-6il/4.5i -O -Bxa1g3+l"Wavelength (km)" -Byafg+l"Admittance (mGal/km)" -BWSne -Sc0.05i -Ey0.2c/0.5p,red -Gred -K -i0,1s1000,2s1000 -Y5.3i >> $ps
gmt psxy cross.txt -R -J -O -K -Sp -Ey0.1c/0.5p,green -i0,11s1000,12s1000 >> $ps
gmt psxy cross.txt -R -J -O -W0.25p,green -i0,11s1000 >> $ps
