#!/usr/bin/env bash
#
# Check that gmtspatial -N works for various modifiers
# -Npoly : Write out features who fully or partially are inside polygon
# -Npoly+z : Same, but place polygon ID as last column per row
# -Npoly+r : Same, just report if fully or partially are inside polygon
# -Npoly+a : Write out features who are fully inside polygon
# -Npoly+z+a : Same, but place polygon ID as last column per row
# -Npoly+r+a : Same, but just report if fully inside polygon
# The polygon
cat << EOF > test_poly.txt
> -Z999
140 10
140 20
150 20
150 10
140 10
EOF
# The features
cat << EOF > test_points.txt
> feature 1 crosses into the polygon from the outside
138 15
139 15
140 15
141 15
142 15
143 15
> feature 2 is completely outside
138 12
139 12
> feature 3 is completely inside
141 18
142 18
EOF

echo "# Check -Ntest_poly.txt [Feature 1 and 3 expected]" > result
gmt spatial test_points.txt -Ntest_poly.txt >> result
echo "# Check -Ntest_poly.txt+z [Feature 1 and 3 expected]" >> result
gmt spatial test_points.txt -Ntest_poly.txt+z >> result
echo "# Check -Ntest_poly.txt+r [Feature 1 and 3 expected]" >> result
gmt spatial test_points.txt -Ntest_poly.txt+r >> result
echo "# Check -Ntest_poly.txt+a [Feature 3 expected]" >> result
gmt spatial test_points.txt -Ntest_poly.txt+a >> result
echo "# Check -Ntest_poly.txt+z+a [Feature 3 expected]" >> result
gmt spatial test_points.txt -Ntest_poly.txt+z+a >> result
echo "# Check -Ntest_poly.txt+r+a [Feature 3 expected]" >> result
gmt spatial test_points.txt -Ntest_poly.txt+r+a >> result

# This is the verified expected outputs:
cat << EOF > answer
# Check -Ntest_poly.txt [Feature 1 and 3 expected]
> feature 1 crosses into the polygon from the outside -Z999
138	15
139	15
140	15
141	15
142	15
143	15
138	15
> feature 3 is completely inside -Z999
141	18
142	18
# Check -Ntest_poly.txt+z [Feature 1 and 3 expected]
> feature 1 crosses into the polygon from the outside
138	15	999
139	15	999
140	15	999
141	15	999
142	15	999
143	15	999
138	15	999
> feature 3 is completely inside
141	18	999
142	18	999
# Check -Ntest_poly.txt+r [Feature 1 and 3 expected]
3 points from table 0 segment 0 is inside polygon # 999
2 points from table 0 segment 2 is inside polygon # 999
# Check -Ntest_poly.txt+a [Feature 3 expected]
> feature 3 is completely inside -Z999
141	18
142	18
# Check -Ntest_poly.txt+z+a [Feature 3 expected]
> feature 3 is completely inside
141	18	999
142	18	999
# Check -Ntest_poly.txt+r+a [Feature 3 expected]
All points from table 0 segment 2 is inside polygon # 999
EOF
diff -q --strip-trailing-cr answer result > fail
