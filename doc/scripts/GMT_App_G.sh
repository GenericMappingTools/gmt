#!/bin/sh
#	$Id: GMT_App_G.sh,v 1.2 2001-04-03 16:06:39 pwessel Exp $
#
#	Makes the insert for Appendix G (fonts)
#

# dy is line spacing and y0 is total box height

dy=-0.2222
y0=4.744
#../../rsccs_refresh.sh PS_font_names.h
cp -f ../../src/PS_font_names.h .

$AWK -F\" '{print $2}' PS_font_names.h > t

gmtset FRAME_PEN 0.5p
psxy -R0/5.4/0/$y0 -Jx1i -P -K -B0 -M <<EOF> GMT_App_G.ps
>
0.3	0
0.3	$y0
>
2.7	0
2.7	$y0
>
3	0
3	$y0
EOF
psxy -R -Jx -O -K -Y${y0}i /dev/null >> GMT_App_G.ps
pstext -R -Jx -O -K -Y${dy}i <<EOF>> GMT_App_G.ps
0.15	0.05	10	0	0	BC	\\043
1.55	0.05	10	0	0	BC	Font Name
2.85	0.05	10	0	0	BC	\\043
4.15	0.05	10	0	0	BC	Font Name
EOF
psxy -R -Jx -O -K <<EOF>> GMT_App_G.ps
0	0
5.4	0
EOF

i=1
while [ $i -le 19 ]
do
	i1=`echo "$i - 1" | bc`
	i2=`echo "$i1 + 19" | bc`
	k1=$i
	k2=`echo "$i + 19" | bc`

	f1=`sed -n ${k1}p t`
	f2=`sed -n ${k2}p t`

	if [ $i1 -eq "12" ]; then
		f1="Symbol @%0%(Symbol)@%%"
	fi
	fn2=$i2
	if [ $i2 -eq "34" ]; then
		f2="ZapfDingbats @%0%(ZapfDingbats)@%%"
		fn2=0
	else if [ $i2 -gt "34" ]; then
		f2="$f2 @%0%(JPN)@%%"
		fn2=0
	     fi
	fi

	pstext -R -Jx -O -K -Y${dy}i <<EOF>> GMT_App_G.ps
0.15	0.03	10	0	$i1	BC	$i1
0.4	0.03	10	0	$i1	BL	$f1
2.85	0.03	10	0	$fn2	BC	$i2
3.1	0.03	10	0	$i2	BL	$f2
EOF
	i=`echo "$i + 1" | bc`
done

f1=`sed -n 39p t`
pstext -R -Jx -O -K -Y${dy}i << EOF >> GMT_App_G.ps
2.85	0.03	10	0	0	BC	38
3.1	0.03	10	0	0	BL	@%38%$f1@%% (JPN)
EOF

psxy -R -Jx -O /dev/null >> GMT_App_G.ps
\rm -f t PS_font_names.h
gmtset FRAME_PEN 1.25p
