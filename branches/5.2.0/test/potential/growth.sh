#!/bin/bash
#	$Id$
#
# Test the output of gmt grdseamount for evolving Gaussian shapes
ps=growth.ps
m=g
f=0.2
gmt gmtset MAP_FRAME_TYPE plain
cat << EOF > t.txt
#lon lat azimuth, semi-major, semi-minor, height tstart tend
50	75	-20	60	30	5000	6	1
150	75	30	50	40	4000	10	4
EOF
grdseamount -Rk0/200/0/150 -I1000 -Gsmt_%05.2f.nc t.txt -T10/0/0.25 -Qc/l -Dk -E -F$f -C$m -Z-1
grdseamount -Rk0/200/0/150 -I1000 -Gsmt.nc t.txt -C$m -Dk -E -F$f -Z-1
grdcontour smt_00.00.nc+Uk -Jx0.03i -Xc -P -C500 -A1000 -GlLM/RM -Bafg -K > $ps
grdcontour smt.nc+Uk -J -O -K -C500 -A1000 -Gl50/20/50/130,150/20/150/130 -Wc0.25p,red -Wa0.75p,red --FONT_ANNOT_PRIMARY=12p,Helvetica,red >> $ps
grdtrack -Gsmt_00.00.nc+Uk -E0/75/200/75 -o0,1 -nn | psxy -R0/200/20/130 -J -O -K -W1p >> $ps
ls smt_*.*.nc > t.lis
makecpt -Cjet -T0/41/1 -N -I > t.cpt
makecpt -Cjet -T0/10/1 -N -I > t2.cpt
psbasemap -R0/200/0/5100 -JX6i/3i -O -K -Y6i -Bafg >> $ps
let k=1
while read file; do
	rgb=`sed -n ${k}p t.cpt | awk '{print $2}'`
	if [ $k -eq 22 ]; then
		rgb=black
	fi
	grdtrack -G${file}+Uk -E0/75/200/75 -o0,2 -nn | psxy -R -J -O -K -G$rgb >> $ps
	let k=k+1
done < t.lis
grdtrack -Gsmt.nc+Uk -E0/75/200/75 -o0,2 -nn | psxy -R -J -O -K -W1p >> $ps
psscale -Ct2.cpt -D3i/-0.4i/-5.5i/0.15ih -O -K -Baf -Bx+l"Time of emplacement" >> $ps
psxy -R -J -O -T >> $ps
rm -f smt_*.??.nc smt.nc t.lis t.cpt t2.cpt t.txt
