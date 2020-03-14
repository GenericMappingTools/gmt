#!/usr/bin/env bash
#
# Test the output of gmt grdflexure for single circular Gaussian seamount on elastic plate with inplane forces
ps=flex_inplane.ps
m=g
f=0.2
N=5e12
Te=10k
gmt set MAP_FRAME_TYPE plain GMT_FFT kiss

cat << EOF > t.txt
#lon lat azimuth, semi-major, semi-minor, height
0	0	0	50	50	5000
EOF
# Make and plot seamount
gmt grdseamount -R-300/300/-200/200+uk -I1000 -Gsmt.nc t.txt -Dk -E -F$f -C$m
gmt grdtrack -Gsmt.nc+Uk -ELM/RM -o0,2 | gmt psxy -R-300/300/0/5500 -JX6i/3i -P -K -W1p -Gbrown -Bafg10000 -BWSne -Xc > $ps
echo "-300 5500 TOPO" | gmt pstext -R -J -O -K -F+jTL+f14p -Dj0.1i >>$ps
# Regular isotropic felxure solution
gmt grdflexure smt.nc -D3300/2700/1000 -E$Te -Gflex_e.nc
gmt grdtrack -Gflex_e.nc+Uk -ELM/RM -o0,2 | gmt psxy -R-300/300/-3500/500 -JX6i/3i -O -K -W1p -BWsne -Bafg10000 -Y3.1i >> $ps
# Apply extension $N
gmt grdflexure smt.nc -D3300/2700/1000 -A${N}/0/0 -E$Te -Gflex_ex.nc
gmt grdtrack -Gflex_ex.nc+Uk -ELM/RM -o0,2 | gmt psxy -R -J -O -K -W1p,blue >> $ps
# Apply compression -$N
gmt grdflexure smt.nc -D3300/2700/1000 -A-${N}/0/0 -E$Te -Gflex_co.nc
gmt grdtrack -Gflex_co.nc+Uk -ELM/RM -o0,2 | gmt psxy -R -J -O -K -W1p,red >> $ps
echo "-300 -600 FLEX [T@-e@- = $Te]" | gmt pstext -R -J -O -K -F+jLB+f14p -Dj0.1i >>$ps
# Plot gravity of compensation (dashed) as well as total (solid)
gmt gravfft smt.nc -Gsmt_grav.nc -Nf+a -Ff -E4 -D1700 -W6k
gmt gravfft flex_e.nc -Ggrav_e.nc -Nf+a -Ff -E2 -D600 -W13k
gmt gravfft flex_ex.nc -Ggrav_ex.nc -Nf+a -Ff -E2 -D600 -W13k
gmt gravfft flex_co.nc -Ggrav_co.nc -Nf+a -Ff -E2 -D600 -W13k
gmt grdmath smt_grav.nc grav_e.nc ADD = tmp.nc
gmt grdtrack -Gtmp.nc+Uk -ELM/RM -o0,2 | gmt psxy -R-300/300/-60/280 -JX6i/3i -O -K -W1p -BWsne -Bafg10000 -Y3.1i >> $ps
gmt grdtrack -Ggrav_e.nc+Uk -ELM/RM -o0,2 | gmt psxy -R -J -O -K -W0.25,-  >> $ps
gmt grdmath smt_grav.nc grav_ex.nc ADD = tmp.nc
gmt grdtrack -Gtmp.nc+Uk -ELM/RM -o0,2 | gmt psxy -R -J -O -K -W1p,blue  >> $ps
gmt grdtrack -Ggrav_ex.nc+Uk -ELM/RM -o0,2 | gmt psxy -R -J -O -K -W0.25,blue,-  >> $ps
gmt grdmath smt_grav.nc grav_co.nc ADD = tmp.nc
gmt grdtrack -Gtmp.nc+Uk -ELM/RM -o0,2 | gmt psxy -R -J -O -K -W1p,red  >> $ps
gmt grdtrack -Ggrav_co.nc+Uk -ELM/RM -o0,2 | gmt psxy -R -J -O -K -W0.25,red,-  >> $ps
echo "-300 270 FAA" | gmt pstext -R -J -O -K -F+jLT+f14p -Dj0.1i >>$ps
gmt pstext -R -J -O -K -F+jRB+f14p -Dj0.1i << EOF >>$ps
300 240 Isotropic [0 Pa]
300 210 @;blue;Extension [$N Pa]@;;
300 180 @;red;Compression [-$N Pa]@;;
EOF
gmt psxy -R -J -O -T >> $ps
