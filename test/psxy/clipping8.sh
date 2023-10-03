#!/usr/bin/env bash
# Shows the essence of issue # 949.  The problem is related
# to the test in gmt_map.c in map_wesn_clip near #L1738.
# That section finds that one end of the badpol.txt polygon
# is > 180 degrees away and subtracts 360, thus creating
# another polygon that does cross the border, leading to
# the long-way polygon entering into the mix.

ps=clipping8.ps
generate=0	# When 1 this fake-generates the correct solution to give PS to check against
cat << EOF > badpol.txt
90	-10
130	-10
130	+10
90	+10
EOF
scl=$(gmt math -Q 6 360 DIV =)
xa=$(gmt math -Q 96 -110 SUB $scl MUL =)
xb=$(gmt math -Q 130 -110 SUB $scl MUL =)
gmt psxy -R-120/160/0/9 -Jx${scl}i/1i -P -K -W0.25p,- -X1.5i << EOF > $ps
>
90	0
90	9
>
130	0
130	9
EOF
x=$(gmt math -Q $scl 10 MUL =)
gmt psxy -R-120/80/-15/+15 -Jm${scl}i -Gred badpol.txt -Baf -BWSne -A -O -K >> $ps
gmt psxy -R-110/90/-15/+15 -Jm${scl}i -Gred badpol.txt -Baf -BWSne -A -O -K -X${x}i -Y0.95i >> $ps
if [ $generate -eq 1 ]; then
	gmt psxy -R-100/100/-15/+15 -Jm${scl}i -Gred -Baf -BWSne -A -O -K -X${x}i -Y0.95i <<- EOF >> $ps
	90	-10
	100	-10
	100	10
	90	10
	EOF
	gmt psxy -R-90/110/-15/+15 -Jm${scl}i -Gred -Baf -BWSne -A -O -K -X${x}i -Y0.95i <<- EOF >> $ps
	90	-10
	110	-10
	110	10
	90	10
	EOF
	gmt psxy -R-80/120/-15/+15 -Jm${scl}i -Gred -Baf -BWSne -A -O -K -X${x}i -Y0.95i <<- EOF >> $ps
	90	-10
	120	-10
	120	10
	90	10
	EOF
else
	gmt psxy -R-100/100/-15/+15 -Jm${scl}i -Gred badpol.txt -Baf -BWSne -A -O -K -X${x}i -Y0.95i >> $ps
	gmt psxy -R-90/110/-15/+15 -Jm${scl}i -Gred badpol.txt -Baf -BWSne -A -O -K -X${x}i -Y0.95i >> $ps
	gmt psxy -R-80/120/-15/+15 -Jm${scl}i -Gred badpol.txt -Baf -BWSne -A -O -K -X${x}i -Y0.95i >> $ps
fi
gmt psxy -R-70/130/-15/+15 -Jm${scl}i -Gred badpol.txt -Baf -BWSne -A -O -K -X${x}i -Y0.95i >> $ps
gmt psxy -R-60/140/-15/+15 -Jm${scl}i -Gred badpol.txt -Baf -BWSne -A -O -K -X${x}i -Y0.95i >> $ps
gmt psxy -R-50/150/-15/+15 -Jm${scl}i -Gred badpol.txt -Baf -BWSne -A -O -K -X${x}i -Y0.95i >> $ps
gmt psxy -R-40/160/-15/+15 -Jm${scl}i -Gred badpol.txt -Baf -BWSne -A -O -K -X${x}i -Y0.95i >> $ps
gmt psxy -R-30/170/-15/+15 -Jm${scl}i -Gred badpol.txt -Baf -BWSne -A -O -X${x}i -Y0.95i >> $ps
