#!/bin/bash
#	$Id: GMT_App_M_1.sh,v 1.4 2011-02-28 00:58:01 remko Exp $
#
#	Makes the insert for Appendix M(cpt)
#
. functions.sh

trap 'rm -f $$.*; exit 1' 1 2 3 15

grep -v '#' ../../share/conf/gmt_cpt.conf | cut -d: -f1 | sort -r > $$.lis

ps=GMT_App_M_1.ps
n=`cat $$.lis | wc -l`

# dy is line spacing and y0 is total box height

gmtset FRAME_PEN thinner ANNOT_FONT_SIZE_PRIMARY 8p TICK_LENGTH 0.1i ANNOT_OFFSET_PRIMARY 0.04i
psbasemap -R0/6.1/0/6.9 -Jx1i -P -K -B0 > $ps

i=1
y=0.475
y2=0.35
dy=0.56
while [ $i -le 24 ]
do
	j=`expr $i + 1`
	left=`sed -n ${j}p $$.lis`
	right=`sed -n ${i}p $$.lis`
	makecpt -C$left -Z > $$.left.cpt
	makecpt -C$left -T-1/1/0.25 > $$.left2.cpt
	makecpt -C$right -Z > $$.right.cpt
	makecpt -C$right -T-1/1/0.25 > $$.right2.cpt
	psscale -D1.55/$y/2.70/0.125h -C$$.left.cpt -B0 -O -K >> $ps
	psscale -D4.50/$y/2.70/0.125h -C$$.right.cpt -B0 -O -K >> $ps
	psscale -D1.55/$y2/2.70/0.125h -C$$.left2.cpt -Bf0.25 -O -K >> $ps
	psscale -D4.50/$y2/2.70/0.125h -C$$.right2.cpt -Bf0.25 -O -K >> $ps
	echo 1.55 $y 9 0 1 BC ${left} | pstext -R -J -O -K -D0/0.05 >> $ps
	echo 4.50 $y 9 0 1 BC ${right} | pstext -R -J -O -K -D0/0.05 >> $ps
	i=`expr $i + 2`
	y=`gmtmath -Q $y $dy ADD =`
	y2=`gmtmath -Q $y2 $dy ADD =`
done

psxy -R -J -O /dev/null >> $ps
rm -f $$.*
