#!/bin/bash
#	$Id: GMT_App_M_1.sh,v 1.6 2011-05-01 18:06:37 remko Exp $
#
#	Makes the insert for Appendix M(cpt)
#
. ./functions.sh

grep -v '#' ../../share/conf/gmt_cpt.conf | cut -d: -f1 | sort -r > $$.lis

ps=GMT_App_M_1.ps
n=`cat $$.lis | wc -l`

# dy is line spacing and y0 is total box height

gmtset MAP_FRAME_PEN thinner FONT_ANNOT_PRIMARY 8p MAP_TICK_LENGTH 0.1i MAP_ANNOT_OFFSET_PRIMARY 0.04i
psbasemap -R0/6.1/0/6.9 -Jx1i -P -K -B0 > $ps

i=1
y=0.475
y2=0.35
dy=0.52
while [ $i -le 26 ]
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
	pstext -R -J -O -K -D0/0.05 -F+f9p,Helvetica-Bold+jBC >> $ps <<- END
	1.55 $y ${left}
	4.50 $y ${right}
	END
	i=`expr $i + 2`
	y=`gmtmath -Q $y $dy ADD =`
	y2=`gmtmath -Q $y2 $dy ADD =`
done

psxy -R -J -O /dev/null >> $ps
