#!/usr/bin/env bash
#
#	Makes the insets for Appendix M(cpt)
#	[skip srtm which is just a special version of dem2]
#
# Use the knowledge that we need 3 pages: First two pages are the
# 44 original GMT 5 CPTs and the last page has 24 scientific colormaps
# from Fabio [www.fabiocrameri.ch/visualisation]

GMT_SHAREDIR=$(gmt --show-sharedir)

cat << EOF > skip.lis
acton
bamako
batlow
berlin
bilbao
broc
buda
cork
davos
devon
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
vik
srtm
EOF

#sed -e 's/"//g' "${GMT_SOURCE_DIR}"/src/gmt_cpt_masters.h | grep -v srtm | awk '{print $1}' | sort -r > tt.lis
sed -e 's/"//g' "${GMT_SOURCE_DIR}"/src/gmt_cpt_masters.h | fgrep -v -f skip.lis | awk '{print $1}' | sort -r > tt.lis

n=$(cat tt.lis | wc -l)
let n2=n/2
let n2=22
# dy is line spacing and y0 is total box height
dy=0.75
y0=$(gmt math -Q $n2 2 ADD $dy MUL 0.5 MUL =)

gmt begin GMT_App_M_1a
gmt set MAP_FRAME_PEN thinner FONT_ANNOT_PRIMARY 8p MAP_TICK_LENGTH_PRIMARY 0.1i MAP_ANNOT_OFFSET_PRIMARY 0.04i
gmt basemap -R0/6.1/0/$y0 -Jx1i -B0

let i=1+n2
y=0.475
y2=0.35
while [ $i -le $n ]
do
	j=$(expr $i + 1)
	left=$(sed -n ${j}p tt.lis)
	right=$(sed -n ${i}p tt.lis)
	if [ "$left" = "categorical" ]; then
		gmt makecpt -H -C$left > tt.left.cpt
	else
		gmt makecpt -H -C$left -T-1/1 > tt.left.cpt
	fi
	gmt makecpt -H -C$left -T-1/1/0.25 > tt.left2.cpt
	if [ "$right" = "categorical" ]; then
		gmt makecpt -H -C$right > tt.right.cpt
	else
		gmt makecpt -H -C$right -T-1/1 > tt.right.cpt
	fi
	gmt makecpt -H -C$right -T-1/1/0.25 > tt.right2.cpt
	gmt colorbar -Dx1.55i/${y}i+w2.70i/0.125i+h+jTC -Ctt.left.cpt -B0
	gmt colorbar -Dx4.50i/${y}i+w2.70i/0.125i+h+jTC -Ctt.right.cpt -B0
	gmt colorbar -Dx1.55i/${y2}i+w2.70i/0.125i+h+jTC -Ctt.left2.cpt -Bf0.25
	gmt colorbar -Dx4.50i/${y2}i+w2.70i/0.125i+h+jTC -Ctt.right2.cpt -Bf0.25
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

gmt end show
