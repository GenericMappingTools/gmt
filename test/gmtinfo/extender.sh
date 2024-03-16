#!/usr/bin/env bash
# Test -I rounding and extension
cat << EOF > data.txt
-125	37.6
-111.1	49.2
-121.49	42.5
EOF
cat << EOF > answer.txt
-R-125/-111.1/37.6/49.2
-R-125/-111/37.5/49.5
-R-126/-110/36.5/50.5
-R-125/-111/37/50
-R-126/-110/37/50
EOF
# 0. Exact extent
gmt info data.txt -Ie > result.txt
# 1. Round to nearest 0.5
gmt info data.txt -I0.5 >> result.txt
# 2. Extend outward by 1, then round to nearest 0.5
gmt info data.txt -I0.5+R1 >> result.txt
# 3. Round to nearest 0.5, then round to nearest 1
gmt info data.txt -I0.5+r1 >> result.txt
# 4. Round to nearest 0.5, then extend by 1
gmt info data.txt -I0.5+e1 >> result.txt
# Compare
diff result.txt answer.txt --strip-trailing-cr > fail
