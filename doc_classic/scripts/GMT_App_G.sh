#!/usr/bin/env bash
#
#	Makes the inset for Appendix G (fonts)
#
# dy is line spacing and y0 is total box height

dy=-0.2222
y0=4.3
tr '",' '  ' < "${GMT_SOURCE_DIR}"/src/standard_adobe_fonts.h | awk '{print $2}' > tt.d
gmt set MAP_FRAME_PEN thinner
gmt psxy -R0/5.4/0/$y0 -Jx1i -P -K -B0 <<EOF > GMT_App_G.ps
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
gmt psxy -R -J -O -K -Y${y0}i -T >> GMT_App_G.ps
gmt pstext -R -J -O -K -Y${dy}i -F+f10p+jBC <<EOF >> GMT_App_G.ps
0.15	0.05	\\043
1.55	0.05	Font Name
2.85	0.05	\\043
4.15	0.05	Font Name
EOF
gmt psxy -R -J -O -K <<EOF >> GMT_App_G.ps
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

	f1=`sed -n ${k1}p tt.d`
	f2=`sed -n ${k2}p tt.d`

	if [ $i1 -eq "12" ]; then
		f1="Symbol @%0%(Symbol)@%%"
	fi
	fn2=$i2
	gmt pstext -R -J -O -K -Y${dy}i -F+f+j <<EOF >> GMT_App_G.ps
0.15	0.03	10p,$i1		BC	$i1
0.4	0.03	10p,$i1		BL	$f1
2.85	0.03	10p,$fn2	BC	$i2
3.1	0.03	10p,$i2		BL	$f2
EOF
	i=`echo "$i + 1" | bc`
done

gmt pstext -R -J -O -K -Y${dy}i -F+f+j <<EOF >> GMT_App_G.ps
2.85	0.03	10p,Helvetica		BC	34
3.1	0.03	10p,ZapfDingbats	BL	ZapfDingbats @%0%(ZapfDingbats)@%%
EOF

gmt psxy -R -J -O -T >> GMT_App_G.ps
