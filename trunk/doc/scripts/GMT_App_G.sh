#!/bin/bash
#	$Id: GMT_App_G.sh,v 1.9 2011-02-28 00:58:03 remko Exp $
#
#	Makes the insert for Appendix G (fonts)
#
. functions.sh

trap 'rm -f $$.*; exit 1' 1 2 3 15

# dy is line spacing and y0 is total box height

dy=-0.2222
y0=4.3
grep -v '^#' ../../share/pslib/PS_font_info.d | awk '{print $1}' > $$.d
gmtset FRAME_PEN thinner
psxy -R0/5.4/0/$y0 -Jx1i -P -K -B0 -m <<EOF > GMT_App_G.ps
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
psxy -R -J -O -K -Y${y0}i /dev/null >> GMT_App_G.ps
pstext -R -J -O -K -Y${dy}i <<EOF >> GMT_App_G.ps
0.15	0.05	10	0	0	BC	\\043
1.55	0.05	10	0	0	BC	Font Name
2.85	0.05	10	0	0	BC	\\043
4.15	0.05	10	0	0	BC	Font Name
EOF
psxy -R -J -O -K <<EOF >> GMT_App_G.ps
0	0
5.4	0
EOF

i=1
while [ $i -le 17 ]
do
	i1=`echo "$i - 1" | bc`
	i2=`echo "$i1 + 17" | bc`
	k1=$i
	k2=`echo "$i + 17" | bc`

	f1=`sed -n ${k1}p $$.d`
	f2=`sed -n ${k2}p $$.d`

	if [ $i1 -eq "12" ]; then
		f1="Symbol @%0%(Symbol)@%%"
	fi
	fn2=$i2
	pstext -R -J -O -K -Y${dy}i <<EOF >> GMT_App_G.ps
0.15	0.03	10	0	$i1	BC	$i1
0.4	0.03	10	0	$i1	BL	$f1
2.85	0.03	10	0	$fn2	BC	$i2
3.1	0.03	10	0	$i2	BL	$f2
EOF
	i=`echo "$i + 1" | bc`
done

pstext -R -J -O -K -Y${dy}i <<EOF >> GMT_App_G.ps
2.85	0.03	10	0	0	BC	34
3.1	0.03	10	0	34	BL	ZapfDingbats @%0%(ZapfDingbats)@%%
EOF

psxy -R -J -O /dev/null >> GMT_App_G.ps
rm -f $$.*
