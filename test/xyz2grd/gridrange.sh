#!/usr/bin/env bash
# Check that +sa and +oa parsing work
cat << EOF > answer.txt
1.00000000	1.00000000	-0.50196850
0.00000000	0.00000000	1.25000000
EOF
(echo 0 0 1.25; echo 1 1 -0.5) | gmt xyz2grd -R-1/1/-1/1 -I1 -Gout.nc=nb+sa+oa
gmt grd2xyz -s out.nc --FORMAT_FLOAT_OUT=%.8f > result.txt
diff -q --strip-trailing-cr result.txt answer.txt > fail
