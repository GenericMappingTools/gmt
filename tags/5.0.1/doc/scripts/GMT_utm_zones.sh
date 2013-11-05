#!/bin/bash
#	$Id$
#
# Makes a plot of the global UTM zone grid including the exceptions near Norway/Spitsbergen
. ./functions.sh

pscoast -Rd -JQ9i -Groyalblue -Sazure -Dl -A2000 -B60f6/0wsNe -K --MAP_FRAME_TYPE=plain \
  --FORMAT_GEO_MAP=dddF --MAP_TITLE_OFFSET=0.25i --MAP_ANNOT_OFFSET_PRIMARY=0.15i --FONT_TITLE=24p --FONT_ANNOT_PRIMARY=10p > GMT_utm_zones.ps
cat << EOF > $$.z.d
>  Do S pole zone
-180	-80
   0	-80
+180	-80
>
0	-90
0	-80
>  Do N pole zone
-180	84
   0	84
+180	84
>
0	90
0	84
EOF
gmtmath -T-174/174/6 T 0 MUL = $$.x.d
echo '-90' > $$.L.d
let s=-80
rm -f $$.y.d
while [ $s -lt 72 ]; do
	echo $s >> $$.L.d
	n=$(($s+8))
	cat <<- EOF >> $$.z.d
	> Lat = $s
	-180	$s
	   0	$s
	+180	$s
	EOF
	if [ $s -eq 56 ]; then
		awk '{if ($1 == 6) {print 3} else {print $0}}' $$.x.d > $$.sp.d
	else
		cat $$.x.d > $$.sp.d
	fi
	awk '{printf "> \n%s\t%s\n%s\t%s\n", $1, "'$s'", $1, "'$n'"}' $$.sp.d >> $$.z.d
	gmtmath -Q $s $n ADD 2 DIV = >> $$.y.d
	s=$n
done
echo $n >> $$.L.d
echo '84' >> $$.L.d
echo '90' >> $$.L.d
echo 78 >> $$.y.d
cat << EOF > $$.n.d
C
D
E
F
G
H
J
K
L
M
N
P
Q
R
S
T
U
V
W
X
EOF
n=84
cat << EOF >> $$.z.d
> Lat = $s
-180	$s
   0	$s
+180	$s
EOF
awk '{if ($1 <=0 || $1 >=42) print $0}' $$.x.d > $$.sp.d
cat << EOF >> $$.sp.d
9
21
33
EOF
awk '{printf "> \n%s\t%s\n%s\t%s\n", $1, "'$s'", $1, "'$n'"}' $$.sp.d >> $$.z.d
psxy -R -J -O -K -W0.5p -Ap $$.z.d >> GMT_utm_zones.ps
paste $$.y.d $$.n.d | awk '{printf "180 %s %s\n", $1, $2}' | pstext -R -J -O -K -N -D0.1i/0 -F+f10p,Helvetica-Bold >> GMT_utm_zones.ps
pstext -R -J -O -K -N -F+f10p,Helvetica-Bold << EOF >> GMT_utm_zones.ps
-90	-85	A
+90	-85	B
-90	87	Y
+90	87	Z
EOF
gmtmath -T-180/174/6 T 3 ADD = | awk '{if (NR > 1) printf "%s -90 %d\n", $2, NR}' | pstext -R -J -O -K -N -D0/-0.07i -F+f8p,Times-Italic+jCT >> GMT_utm_zones.ps
gmtmath -T-180/174/6 T 3 ADD = | awk '{if (NR > 1) printf "%s 90 %d\n", $2, NR}' | pstext -R -J -O -K -N -D0/0.07i -F+f8p,Times-Italic+jCB >> GMT_utm_zones.ps
pstext -R -J -O -K -D0/0.025i -F+f8p,Times-Italic+jCB << EOF >> GMT_utm_zones.ps
4.5	72	31X
15	72	33X
27	72	35X
37.5	72	37X
EOF
awk '{if ($1 < 0) printf "-180 %s %s\\232S\n", $1, -$1}' $$.L.d | pstext -R -J -O -K -N -D-0.05i/0 -F+f10p+jRM >> GMT_utm_zones.ps
awk '{if ($1 > 0) printf "-180 %s %s\\232N\n", $1,  $1}' $$.L.d | pstext -R -J -O -K -N -D-0.05i/0 -F+f10p+jRM >> GMT_utm_zones.ps
echo "-180 0 0\\232" | pstext -R -J -O -K -N -D-0.05i/0 -F+f10p+jRM >> GMT_utm_zones.ps
psxy -R -J -O -T >> GMT_utm_zones.ps
rm -f $$.*.d
