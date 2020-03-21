#!/usr/bin/env bash
#
# Makes a plot of the global UTM zone grid including the exceptions near Norway/Spitsbergen
#
gmt begin GMT_utm_zones
gmt set MAP_FRAME_TYPE plain FORMAT_GEO_MAP dddF MAP_TITLE_OFFSET 0.25i MAP_ANNOT_OFFSET_PRIMARY 0.15i FONT_TITLE 24p FONT_ANNOT_PRIMARY 10p PS_MEDIA 11ix8.5i

gmt coast -Rd -JQ9i -Groyalblue -Sazure -Dl -A2000 -Bx60f6 -By0 -BwsNe
cat << EOF > tt.z.d
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
gmt math -T-174/174/6 T 0 MUL = tt.x.d
echo '-90' > tt.L.d
let s=-80
rm -f tt.y.d
while [ $s -lt 72 ]; do
	echo $s >> tt.L.d
	n=$(($s+8))
	cat <<- EOF >> tt.z.d
	> Lat = $s
	-180	$s
	   0	$s
	+180	$s
	EOF
	if [ $s -eq 56 ]; then
		$AWK '{if ($1 == 6) {print 3} else {print $0}}' tt.x.d > tt.sp.d
	else
		cat tt.x.d > tt.sp.d
	fi
	$AWK '{printf "> \n%s\t%s\n%s\t%s\n", $1, "'$s'", $1, "'$n'"}' tt.sp.d >> tt.z.d
	gmt math -Q $s $n ADD 2 DIV = >> tt.y.d
	s=$n
done
echo $n >> tt.L.d
echo '84' >> tt.L.d
echo '90' >> tt.L.d
echo 78 >> tt.y.d
cat << EOF > tt.n.d
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
cat << EOF >> tt.z.d
> Lat = $s
-180	$s
   0	$s
+180	$s
EOF
$AWK '{if ($1 <=0 || $1 >=42) print $0}' tt.x.d > tt.sp.d
cat << EOF >> tt.sp.d
9
21
33
EOF
$AWK '{printf "> \n%s\t%s\n%s\t%s\n", $1, "'$s'", $1, "'$n'"}' tt.sp.d >> tt.z.d
gmt plot -W0.5p -Ap tt.z.d
paste tt.y.d tt.n.d | $AWK '{printf "180 %s %s\n", $1, $2}' | gmt text -N -D0.1i/0 -F+f10p,Helvetica-Bold
gmt text -N -F+f10p,Helvetica-Bold << EOF
-90	-85	A
+90	-85	B
-90	87	Y
+90	87	Z
EOF
gmt math -T-180/174/6 T 3 ADD = | $AWK '{printf "%s -90 %d\n", $2, NR}' | gmt text -N -D0/-0.07i -F+f8p,Times-Italic+jCT
gmt math -T-180/174/6 T 3 ADD = | $AWK '{printf "%s 90 %d\n", $2, NR}' | gmt text -N -D0/0.07i -F+f8p,Times-Italic+jCB
gmt text -D0/0.025i -F+f8p,Times-Italic+jCB << EOF
4.5	72	31X
15	72	33X
27	72	35X
37.5	72	37X
EOF
$AWK '{if ($1 < 0) printf "-180 %s %s@.S\n", $1, -$1}' tt.L.d | gmt text -N -D-0.05i/0 -F+f10p+jRM
$AWK '{if ($1 > 0) printf "-180 %s %s@.N\n", $1,  $1}' tt.L.d | gmt text -N -D-0.05i/0 -F+f10p+jRM
echo "-180 0 0@." | gmt text -N -D-0.05i/0 -F+f10p+jRM
gmt end show
