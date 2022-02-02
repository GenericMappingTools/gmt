#!/usr/bin/env bash
#
# Compare gmt gravfft and gmt grdfft calculations of admittance and coherence between bathymetry and geoid over the Azores
# DVC_TEST
ps=cross_spec.ps
gmt set GMT_FFT kiss
gmt gravfft @bathy_1m.nc @geoid_1m.nc -N+d -Iwck -fg > coh.txt
gmt gravfft @bathy_1m.nc @geoid_1m.nc -N+d -Iwk  -fg > adm.txt
gmt grdfft  @bathy_1m.nc @geoid_1m.nc -N+d -E+wk  -fg > cross.txt

# coh: gmt gravfft in red, gmt grdfft in green
gmt psxy coh.txt -R3/1000/0/1 -JX-6il/4.5i -Bxg3 -Byg -P -Sc0.05i -Gred -Ey0.2c/0.5p,red -K -X1.25i > $ps
gmt psxy cross.txt -R -J -O -K -Sp -Ey0.1c/0.5p,green -i0,15,16 >> $ps
gmt psxy cross.txt -R -J -O -K -W0.25p,green -i0,15 -Bxa1+l"Wavelength (km)" -Byaf+l"Coherence" -BWSne >> $ps
# adm: gmt gravfft in red, gmt grdfft in green
gmt psxy adm.txt -R3/1000/0/2 -JX-6il/4.5i -Bxg3 -Byg -O -Sc0.05i -Ey+w0.2c+p0.5p,red -Gred -K -i0,1+s1000,2+s1000 -Y5.3i >> $ps
gmt psxy cross.txt -R -J -O -K -Sp -Ey+w0.1c+p0.5p,green -i0,11+s1000,12+s1000 >> $ps
gmt psxy cross.txt -R -J -O -W0.25p,green -i0,11+s1000 -Bxa1+l"Wavelength (km)" -Byaf+l"Admittance (mGal/km)" -BWSne >> $ps
