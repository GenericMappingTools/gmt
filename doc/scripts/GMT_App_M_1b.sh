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

# Here we list all the non-categorical/cyclic cpts from the SCM:
cat << EOF > tt.lis
acton
bam
bamako
batlow
batlowK
batlowW
berlin
bilbao
broc
buda
bukavu
cork
davos
devon
fes
grayC
hawaii
imola
lajolla
lapaz
lisbon
nuuk
oleron
oslo
roma
tofino
tokyo
turku
vanimo
vik
EOF

n=$(cat tt.lis | wc -l)
let n2=n/2
let n2=n
# dy is line spacing and y0 is total box height
dy=0.6
y0=$(gmt math -Q $n2 $dy MUL 0.5 MUL 0.1 ADD =)

gmt begin GMT_App_M_1b
gmt set GMT_THEME cookbook
gmt set MAP_FRAME_PEN thinner FONT_ANNOT_PRIMARY 8p MAP_TICK_LENGTH_PRIMARY 0.1i MAP_ANNOT_OFFSET_PRIMARY 0.04i
gmt basemap -R0/6.1/0/$y0 -Jx1i -B0

i=1
y=0.375
y2=0.25
while [ $i -le $n2 ]
do
	j=$(expr $i + 1)
	left=$(sed -n ${j}p tt.lis)
	right=$(sed -n ${i}p tt.lis)
	gmt makecpt -H -C$left -T-1/1 > tt.left.cpt
	gmt makecpt -H -C$left -T-1/1/0.25 > tt.left2.cpt
	gmt makecpt -H -C$right -T-1/1 > tt.right.cpt
	gmt makecpt -H -C$right -T-1/1/0.25 > tt.right2.cpt
	gmt colorbar -Dx1.55i/${y}i+w2.70i/0.125i+h+jTC+e -Ctt.left.cpt -B0
	gmt colorbar -Dx4.50i/${y}i+w2.70i/0.125i+h+jTC+e -Ctt.right.cpt -B0
	gmt colorbar -Dx1.55i/${y2}i+w2.70i/0.125i+h+jTC+e -Ctt.left2.cpt -Bf0.25
	gmt colorbar -Dx4.50i/${y2}i+w2.70i/0.125i+h+jTC+e -Ctt.right2.cpt -Bf0.25
	gmt text -D0/0.05i -F+f9p,Helvetica-Bold+jBC <<- END
	1.55 $y ${left}
	4.50 $y ${right}
	END
	if [ $(grep -c HARD_HINGE ${GMT_SHAREDIR}/cpt/${left}.cpt) -eq 1 ]; then # Plot hard hinge symbol for left CPT
		echo 1.55 $y | gmt plot -St0.2c -Gblack -Wfaint -D0/-0.29i
	elif [ $(grep -c SOFT_HINGE ${GMT_SHAREDIR}/cpt/${left}.cpt) -eq 1 ]; then # Plot soft hinge symbol for left CPT
		echo 1.55 $y | gmt plot -St0.2c -Gwhite -Wfaint -D0/-0.29i
	fi
	if [ $(grep -c HARD_HINGE ${GMT_SHAREDIR}/cpt/${right}.cpt) -eq 1 ]; then # Plot hard hinge symbol for right CPT
		echo 4.50 $y | gmt plot -St0.2c -Gblack -Wfaint -D0/-0.29i
	elif [ $(grep -c SOFT_HINGE ${GMT_SHAREDIR}/cpt/${right}.cpt) -eq 1 ]; then # Plot soft hinge symbol for right CPT
		echo 4.50 $y | gmt plot -St0.2c -Gwhite -Wfaint -D0/-0.29i
	fi
	i=$(expr $i + 2)
	y=$(gmt math -Q $y $dy ADD =)
	y2=$(gmt math -Q $y2 $dy ADD =)
done
rm -f tt.*
gmt end show
