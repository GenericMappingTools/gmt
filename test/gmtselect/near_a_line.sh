#!/bin/sh
#	$Id: near_a_line.sh,v 1.6 2007-06-05 14:02:35 remko Exp $
#
# Making sure both forms of "near a line" works:
# Default (old) behavior is to think of a line as
# a continuous trace of points; hence data points
# within the given distance from the end points are
# considered inside even if they project to the
# extension of the line segment.
# Optional (new) behaviour (-Lp) will only consider
# points near the line if they project inside the
# line's endpoints

echo -n "$0: Test gmtselect's new -L[p] option on given data:	"

# Some test data
grdmath -R0/5/0/5 -I0.1 0 = $$.grd
cat << EOF > $$.d
> line 1
1 1
2 2.2
3 2.8
3.7 4
EOF
grd2xyz $$.grd > $$.xyz
# Do test both with Cartesian and spherical data
# CARTESIAN DATA: distance D = 1 unit
D=1
# Old behavior
psxy -R0/5/0/5 -JX3.25i -P -B1g1WSne -K -Sc0.02 -Gred $$.xyz -M -X0.75i -Y1i > nearline.ps
gmtselect $$.xyz -L${D}/$$.d | psxy -R -J -O -K -Sc0.02 -Ggreen >> nearline.ps
psxy -R -J -O -K $$.d -M -W1p >> nearline.ps
# New behavior
psxy -R -J -O -B1g1WSne -K -Sc0.02 -Gred $$.xyz -M -X3.75i >> nearline.ps
gmtselect $$.xyz -Lp${D}/$$.d | psxy -R -J -O -K -Sc0.02 -Ggreen >> nearline.ps
psxy -R -J -O -K $$.d -M -W1p >> nearline.ps
# SPHERICAL DATA (-fg): distance D = 1 degree ~= 111.13 km
D=111.13
# Old behavior
psxy -R -JM3.25i -O -B1g1WSne -K -Sc0.02 -Gred $$.xyz -M -X-3.75i -Y4i >> nearline.ps
gmtselect $$.xyz -L${D}/$$.d -fg | psxy -R -J -O -K -Sc0.02 -Ggreen >> nearline.ps
psxy -R -J -O -K $$.d -M -W1p >> nearline.ps
# New behavior
psxy -R -J -O -B1g1WSne -K -Sc0.02 -Gred $$.xyz -M -X3.75i >> nearline.ps
gmtselect $$.xyz -Lp${D}/$$.d -fg | psxy -R -J -O -K -Sc0.02 -Ggreen >> nearline.ps
psxy -R -J -O $$.d -M -W1p >> nearline.ps
# gv nearline.ps &
rm -f $$.grd $$.xyz $$.d
compare -density 100 -metric PSNR {,orig/}nearline.ps nearline_diff.png > log
grep inf log > fail
if [ ! -s fail ]; then
	echo "[FAIL]"
	echo $0 >> ../fail_count.d
else
	echo "[PASS]"
	rm -f fail nearline_diff.png log
fi
