#!/bin/bash
#
#	Makes the insets for Appendix M(cpt)
#	[skip srtm which is just a special verison of dem2]
#
# Use the knowledge that we need 3 pages: First two pages are the
# 44 original GMT 5 CPTs and the last page has 17 scientific colormaps
# from Fabio [www.fabiocrameri.ch/visualisation]

cat << EOF > skip.lis
batlow
berlin
bilbao
broc
cork
davos
grayC
lajolla
lapaz
lisbon
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

ps=GMT_App_M_1a.ps
n=`cat tt.lis | wc -l`
let n2=n/2
# dy is line spacing and y0 is total box height
dy=0.75
y0=`echo "$n2 * $dy * 0.5" | bc`

gmt set MAP_FRAME_PEN thinner FONT_ANNOT_PRIMARY 8p MAP_TICK_LENGTH_PRIMARY 0.1i MAP_ANNOT_OFFSET_PRIMARY 0.04i
gmt psbasemap -R0/6.1/0/$y0 -Jx1i -P -K -B0 > $ps

let i=1+n2
y=0.475
y2=0.35
while [ $i -le $n ]
do
	j=`expr $i + 1`
	left=`sed -n ${j}p tt.lis`
	right=`sed -n ${i}p tt.lis`
	gmt makecpt -C$left > tt.left.cpt
	gmt makecpt -C$left -T-1/1/0.25 > tt.left2.cpt
	gmt makecpt -C$right > tt.right.cpt
	gmt makecpt -C$right -T-1/1/0.25 > tt.right2.cpt
	gmt psscale -D1.55i/${y}i+w2.70i/0.125i+h+jTC -Ctt.left.cpt -B0 -O -K >> $ps
	gmt psscale -D4.50i/${y}i+w2.70i/0.125i+h+jTC -Ctt.right.cpt -B0 -O -K >> $ps
	gmt psscale -D1.55i/${y2}i+w2.70i/0.125i+h+jTC -Ctt.left2.cpt -Bf0.25 -O -K >> $ps
	gmt psscale -D4.50i/${y2}i+w2.70i/0.125i+h+jTC -Ctt.right2.cpt -Bf0.25 -O -K >> $ps
	gmt pstext -R -J -O -K -D0/0.05i -F+f9p,Helvetica-Bold+jBC >> $ps <<- END
	1.55 $y ${left}
	4.50 $y ${right}
	END
	i=`expr $i + 2`
	y=`gmt math -Q $y $dy ADD =`
	y2=`gmt math -Q $y2 $dy ADD =`
done

gmt psxy -R -J -O -T >> $ps
