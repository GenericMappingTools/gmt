#!/bin/bash
# Testing firmoviscous flexure code
ps=firmoviscous.ps
# Set densities
rhol=2700
rhom=3300
rhos=2700
rhow=1000
# Create a truncated seamount load or 5km height
gmt grdseamount -Rk-512/511/-512/511 -I1000 -Gsmt.nc+uk -Cg -Dk -E -F0.2 -Z-6000 << EOF
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
while read t; do
	grdflexure smt.nc+uk -D${rhom}/${rhol}/${rhos}/${rhow} -E10k -F2e20 -Gfv_%.12g.nc -Nf+a -T$t -L >> flist
done < times.txt
# Get cross-sections of all solutions
gmtmath -T-200/200/1 0 = t.txt
grdtrack t.txt -G+lflist > a.txt
# Plot flexure(t)
psbasemap -R-200/200/-2000/100 -JX6.5i/2.75i -Bafg10000 -BWSne -P -K --PS_MEDIA=letter > $ps
let col=2
while read t color; do
	let c=2*col
	psxy -R -J a.txt -i0,${col} -W0.5p,$color -O -K >> $ps
	y=`gmtinfo a.txt -C -o${c}`
	echo 0 $y $t | pstext -R -J -O -K -F+f10p,+jCM -Gwhite >> $ps
	let col=col+1
done < times.txt
echo -200 -2000 FLEX | pstext -R -J -O -K -F+f18p+jLB -Dj0.1i >> $ps
# Plot load
psbasemap -R-200/200/-6000/0 -J -O -K -BWsne+glightblue -Baf -Y3i >> $ps
grdtrack -Gsmt.nc t.txt | psxy -R -J -i0,2 -O -K -L+yb -Gbrown >> $ps
echo -200 -6000 LOAD | pstext -R -J -O -K -F+f18p+jLB -Dj0.1i >> $ps
# Calc and plot gravity
psbasemap -R-200/200/-40/200 -J -O -K -BWsne+t"T@-e@- = 10 km   @~\150@~ = 2\26710@+20@+ Pa\267s" -Bafg1000 -Y3i >> $ps
drho=`gmtmath -Q ${rhol} ${rhow} SUB =`
gravfft smt.nc+uk -Gsmt_grav.nc -Nf+a -Ff -E4 -D${drho} -W6k
paste flist times.txt > flist.txt
drho=`gmtmath -Q ${rhom} ${rhos} SUB =`
while read file t color; do
	gravfft ${file}+uk -Gflx_grav.nc -Nf+a -Ff -E2 -D${drho} -W13k
	grdtrack t.txt -Gflx_grav.nc > b.txt
	psxy -R -J -O -K -W0.25p,${color},- b.txt -i0,2 >> $ps
	grdmath smt_grav.nc flx_grav.nc ADD = tmp.nc
	grdtrack t.txt -Gtmp.nc > b.txt
	psxy -R -J -O -K -W0.5p,${color} b.txt -i0,2 >> $ps
done < flist.txt
echo -200 -40 FAA | pstext -R -J -O -K -F+f18p+jLB -Dj0.1i >> $ps
psxy -R -J -O -T >> $ps
