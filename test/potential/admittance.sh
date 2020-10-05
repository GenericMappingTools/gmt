#!/usr/bin/env bash
#
# Compute admittance for synthetic model and data.
ps=admittance.ps
gmt set GMT_FFT kiss
order=4
# 1. Create a bathymetry data set with 3 circular Gaussian seamounts
gmt grdseamount -R0/512/0/512 -I4 -r -Gz.nc -Z-5000 << EOF
310 290 40 3000
290 205 50 4000
210 280 60 2500
EOF

# 2. LL Map: Bathymetry load + flexural contours
gmt makecpt -Crainbow -T-5000/-1000 > z.cpt
gmt grdimage z.nc -Cz.cpt -JX3i -P -Ba -BWSne -K -Y0.75i > $ps
gmt makecpt -Crainbow -T-50/250 > g.cpt

# Compute flexure and overlay on bathymetry
gmt gravfft z.nc+uk -T12000/2800/3300/1000 -Q -Z12000 -N1024/1024+w+a -Gmoho_flex.nc
gmt grdcontour moho_flex.nc -J -O -K -C100 -A500 >> $ps

# 3. LR Map: Gravity from flexure of Moho only
gmt gravfft moho_flex.nc+uk -Ff -E$order -D500 -N+a -Gfaa_flex.nc
gmt grdimage faa_flex.nc -Cg.cpt -J -O -K -Ba -BWSne -X3.5i >> $ps

# 4. ML Map: Gravity from seamounts only
gmt gravfft z.nc+uk -Ff -E$order -D1800 -N+a -Gfaa_z.nc
gmt grdimage faa_z.nc -Cg.cpt -J -O -K -Ba -BWsne -X-3.5i -Y3.25i >> $ps

# 5. MR Map: Total gravity model + 0.5 mGal noise
#gmt grdmath -Rfaa_flex.nc 0 0.5 NRAND = faa_noise.nc
gmt grdmath faa_flex.nc faa_z.nc ADD @faa_noise.nc ADD = faa_total.nc
gmt grdimage faa_total.nc -Cg.cpt -J -O -K -Ba -BWsne -X3.5i >> $ps

# Compute admittance, both data and theoretical, and coherence between topo and gravity
gmt gravfft z.nc+uk faa_total.nc+uk -N+d -Iwkt -Ff -Z12000 -T12000/2800/3300/1000 > adm_t.txt
gmt grdfft z.nc+uk faa_total.nc+uk  -N+d -Er+wk -h+c > adm.txt

# coherence in red, admittance in blue, theoretical admittance in thick lightgray
gmt psxy adm.txt -i0,15,16 -R8/512/0/1 -JX-6il/2.75i -O -K -Sc0.05i \
	-Gred -Ey+w0.2c+p0.5p,red -Bxg3 -X-3.25i -Y3.5i >> $ps
gmt psxy adm.txt -i0,15 -R -J -O -K -W0.5p,red >> $ps
gmt psxy adm.txt -R8/512/0/70 -J -O-Sc0.05i -Ey+w0.2c+p0.5p,blue -Gblue -K -i0,11+s1000,12+s1000 >> $ps
gmt psxy adm.txt -R -J -O -K  -W0.5p,blue -i0,11+s1000 >> $ps
gmt psxy adm_t.txt -R -J -O -K -W0.5p,green -i0,3+s1000 >> $ps
gmt psbasemap -R -J -O -K -Bxa2 -Byaf+l"Admittance (mGal/km)" -BE --FONT_LABEL=16p,Helvetica,blue >> $ps
gmt psbasemap -R8/512/0/1 -J -O -Bxa2+u" km" -Byaf+l"Coherence" -BWSn --FONT_LABEL=16p,Helvetica,red >> $ps
