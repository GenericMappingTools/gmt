#!/usr/bin/env bash
# Testing firmoviscous flexure code
ps=firmoviscous.ps
gmt set GMT_FFT kiss
# Set densities
rhol=2700
rhom=3300
rhos=2700
rhow=1000
# Create a truncated seamount load or 5km height
gmt grdseamount -R-512/511/-512/511+uk -I1000 -Gsmt.nc+uk -Cg -Dk -E -F0.2 -Z-6000 << EOF
0	0	0	40	40	5000	75	73
EOF
# Select times for calculation
cat << EOF > times.txt
3k	red
10k	orange
30k	green
70k	blue
1M	black
EOF
rm -f flist
while read t color; do
	gmt grdflexure smt.nc+uk -D${rhom}/${rhol}/${rhos}/${rhow} -E10k -F2e20 -Gfv_%.12g.nc -Nf+a -T$t -L >> flist
done < times.txt
# Get cross-sections of all solutions
gmt math -T-200/200/1 0 = t.txt
gmt grdtrack t.txt -G+lflist > a.txt
# Plot flexure(t)
gmt psbasemap -R-200/200/-2000/100 -JX6.5i/2.75i -Bafg10000 -BWSne -P -K > $ps
let col=2
while read t color; do
	let c=2*col
	gmt psxy -R -J a.txt -i0,${col} -W0.5p,$color -O -K >> $ps
	y=$(gmt info a.txt -C -o${c})
	echo 0 $y $t | gmt pstext -R -J -O -K -F+f10p,+jCM -Gwhite >> $ps
	let col=col+1
done < times.txt
echo -200 -2000 FLEX | gmt pstext -R -J -O -K -F+f18p+jLB -Dj0.1i >> $ps
# Plot load
gmt psbasemap -R-200/200/-6000/0 -J -O -K -BWsne+glightblue -Baf -Y3i >> $ps
gmt grdtrack -Gsmt.nc t.txt | gmt psxy -R -J -i0,2 -O -K -L+yb -Gbrown >> $ps
echo -200 -6000 LOAD | gmt pstext -R -J -O -K -F+f18p+jLB -Dj0.1i >> $ps
# Calc and plot gravity
gmt psbasemap -R-200/200/-40/200 -J -O -K -BWsne+t"T@-e@- = 10 km   @~\150@~ = 2\26410@+20@+ Pa\264s" -Bafg1000 -Y3i >> $ps
drho=$(gmt math -Q ${rhol} ${rhow} SUB =)
gmt gravfft smt.nc+uk -Gsmt_grav.nc -Nf+a -Ff -E4 -D${drho} -W6k
paste flist times.txt > flist.txt
drho=$(gmt math -Q ${rhom} ${rhos} SUB =)
while read file t t2 color; do
	gmt gravfft ${file}+uk -Gflx_grav.nc -Nf+a -Ff -E2 -D${drho} -W13k
	gmt grdtrack t.txt -Gflx_grav.nc > b.txt
	gmt psxy -R -J -O -K -W0.25p,${color},- b.txt -i0,2 >> $ps
	gmt grdmath smt_grav.nc flx_grav.nc ADD = tmp.nc
	gmt grdtrack t.txt -Gtmp.nc > b.txt
	gmt psxy -R -J -O -K -W0.5p,${color} b.txt -i0,2 >> $ps
done < flist.txt
echo -200 -40 FAA | gmt pstext -R -J -O -K -F+f18p+jLB -Dj0.1i >> $ps
gmt psxy -R -J -O -T >> $ps
