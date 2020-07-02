#!/usr/bin/env bash
#
# Test the output of gmt grdseamount for evolving Gaussian shapes
ps=growth.ps
m=g
f=0.2
gmt set MAP_FRAME_TYPE plain
cat << EOF > t.txt
#lon lat azimuth, semi-major, semi-minor, height tstart tend
50	75	-20	60	30	5000	6	1
150	75	30	50	40	4000	10	4
EOF
gmt grdseamount -R0/200/0/150+uk -I1000 -Gsmt_%05.2f.nc t.txt -T10/0/0.25 -Qc/l -Dk -E -F$f -C$m -Z-1
gmt grdseamount -R0/200/0/150+uk -I1000 -Gsmt.nc t.txt -C$m -Dk -E -F$f -Z-1
gmt grdcontour smt_01.00.nc+Uk -Jx0.03i -Xc -P -C500 -A1000 -GlLM/RM -Bafg -K > $ps
gmt grdcontour smt.nc+Uk -J -O -K -C500 -A1000 -Gl50/20/50/130,150/20/150/130 -Wc0.25p,red -Wa0.75p,red --FONT_ANNOT_PRIMARY=12p,Helvetica,red >> $ps
gmt grdtrack -Gsmt_01.00.nc+Uk -E0/75/200/75 -o0,1 -nn | gmt psxy -R0/200/20/130 -J -O -K -W1p >> $ps
ls smt_*.*.nc > t.lis
gmt makecpt -Cjet -T1/37/1 -N -I > t.cpt
gmt makecpt -Cjet -T1/10/0.25 -N -I > t2.cpt
gmt psbasemap -R0/200/0/5100 -JX6i/3i -O -K -Y6i -Bafg >> $ps
let k=1
while read file; do
	rgb=$(sed -n ${k}p t.cpt | awk '{print $2}')
	if [ $k -eq 18 ]; then
		rgb=black
	fi
	gmt grdtrack -G${file}+Uk -E0/75/200/75 -o0,2 -nn | gmt psxy -R -J -O -K -G$rgb >> $ps
	let k=k+1
done < t.lis
gmt grdtrack -Gsmt.nc+Uk -E0/75/200/75 -o0,2 -nn | gmt psxy -R -J -O -K -W1p >> $ps
gmt psscale -Ct2.cpt -Dx3i/-0.4i+w-6i/0.15i+h+jTC -O -K -Baf -Bx+l"Time of emplacement" >> $ps
gmt psxy -R1/10/0/1 -JX-6i/0.15i -O -K -Y-0.55i -Gblack << EOF >> $ps
5.25	0
5.50	0
5.50	1
5.25	1
EOF
gmt psxy -R -J -O -T >> $ps
