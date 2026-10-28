#!/usr/bin/env bash
#
# Test thin polar (-Jp|P) sectors, such as the single axes one would combine into a radar plot.
# A sector that ends up parallel to the y-axis used to blow the -JP scale up to absurdity and
# to lose all of its annotations.  See https://github.com/GenericMappingTools/gmt/issues/7059

R=-R-0.0001/0.0001/0/1

# 1. The plot length, in cm, of the radial axis r = 0 to 1.
# Note: with -JP the size is the width of the plot, so rotating a thin sector by 45 degrees
# leaves it 12 cm wide but 12/cos(45) = 16.971 cm long.  -Jp<scale> sets the radial scale and
# is therefore not affected by the rotation.
axis_length () {
	printf "0 0\n0 1\n" | gmt mapproject $R "$1" --PROJ_LENGTH_UNIT=cm 2>/dev/null | \
		awk 'NR==1 {x=$1; y=$2} NR==2 {printf "%.3f\n", sqrt (($1-x)^2 + ($2-y)^2)}'
}
cat << EOF > answer.txt
-JP12c 12.000
-JP12c+t45 16.971
-JP12c+t90 12.000
-JP12c+t-90 12.000
-JP12c+du 12.000
-Jp12c 12.000
-Jp12c+t45 12.000
-Jp12c+t90 12.000
EOF
rm -f result.txt
for J in -JP12c -JP12c+t45 -JP12c+t90 -JP12c+t-90 -JP12c+du -Jp12c -Jp12c+t45 -Jp12c+t90; do
	echo "$J $(axis_length $J)" >> result.txt
done
diff result.txt answer.txt > fail

# 2. A full circle with -JP12c is 12 cm across, i.e., the radius is 6 cm
printf "0 0\n0 1\n" | gmt mapproject -R0/360/0/1 -JP12c --PROJ_LENGTH_UNIT=cm 2>/dev/null | \
	awk 'NR==1 {x=$1; y=$2} NR==2 {printf "%.3f\n", sqrt (($1-x)^2 + ($2-y)^2)}' > radius.txt
echo 6.000 > radius_answer.txt
diff radius.txt radius_answer.txt >> fail

# 3. The axis keeps its 6 annotations on both radial edges no matter how it is rotated
cat << EOF > annot_answer.txt
+t0 12
+t-45 12
+t-90 12
+t90 12
+t135 12
EOF
rm -f annot.txt
for t in 0 -45 -90 90 135; do
	echo "+t$t $(gmt psbasemap $R -Jp12c+t$t -Bya -P 2>/dev/null | grep -oE '\) [a-z][a-z] Z' | wc -l | tr -d ' ')" >> annot.txt
done
diff annot.txt annot_answer.txt >> fail
