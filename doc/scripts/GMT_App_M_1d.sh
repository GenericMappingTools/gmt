#!/usr/bin/env bash
#
#	Makes the insets for Appendix M(cpt)
#	[skip srtm which is just a special version of dem2]
#
# We have four sets of CPT figures to make:
# 1a) Our regular, traditional GMT CPTs [44]
# 1b) The regular Scientific Color Maps* [30]
# 1c) Categorical CPTs (ours and SCM*)  [18]
# 1d) Cyclic CPTs from SCM* [5]
#
# *from Fabio [www.fabiocrameri.ch/visualisation]

GMT_SHAREDIR=$(gmt --show-sharedir)

# Here we list all the cyclic cpts from the SCM:
cat << EOF > tt.lis
bamO
brocO
corkO
romaO
vikO
EOF

n=$(cat tt.lis | wc -l)
let n2=n/2
let n2=n
# dy is line spacing and y0 is total box height
dy=0.6
y0=$(gmt math -Q $n2 $dy MUL 0.5 MUL 0.25 ADD =)

gmt begin GMT_App_M_1d
gmt set GMT_THEME cookbook
gmt set MAP_FRAME_PEN thinner FONT_ANNOT_PRIMARY 8p MAP_TICK_LENGTH_PRIMARY 0.1i MAP_ANNOT_OFFSET_PRIMARY 0.04i
gmt basemap -R0/6.1/0/$y0 -Jx1i -B0

i=1
y=0.375
y2=0.25
while [ $i -le $n2 ]
do
	j2=$(expr $n2 + 1 - $i)
	right=$(sed -n ${j2}p tt.lis)
	if [[ ${i} < ${n2} ]]; then
		j1=$(expr $n2 - $i)
		left=$(sed -n ${j1}p tt.lis)
		gmt makecpt -H -C$left -T-1/1 > tt.left.cpt
		gmt makecpt -H -C$left -T-1/1/0.25 > tt.left2.cpt
		gmt colorbar -Dx1.55i/${y}i+w2.70i/0.125i+h+jTC -Ctt.left.cpt -B0
		gmt colorbar -Dx1.55i/${y2}i+w2.70i/0.125i+h+jTC -Ctt.left2.cpt -Bf0.25
		echo 1.55 $y ${left} | gmt text -D0/0.05i -F+f9p,Helvetica-Bold+jBC
		x=4.50
	else
		x=3.05
	fi
	gmt makecpt -H -C$right -T-1/1 > tt.right.cpt
	gmt makecpt -H -C$right -T-1/1/0.25 > tt.right2.cpt
	gmt colorbar -Dx${x}i/${y}i+w2.70i/0.125i+h+jTC -Ctt.right.cpt -B0
	gmt colorbar -Dx${x}i/${y2}i+w2.70i/0.125i+h+jTC -Ctt.right2.cpt -Bf0.25
	echo ${x} $y ${right} | gmt text -D0/0.05i -F+f9p,Helvetica-Bold+jBC
	i=$(expr $i + 2)
	y=$(gmt math -Q $y $dy ADD =)
	y2=$(gmt math -Q $y2 $dy ADD =)
done
rm -f tt.*
gmt end show
