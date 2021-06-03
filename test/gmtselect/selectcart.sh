#!/usr/bin/env bash
#
# Test script used for problem reported bin gmt-user forum message
# https://forum.generic-mapping-tools.org/t/questions-about-gmtselect-in-cartesian-coordinate/1716

# Some test data
cat << EOF > line.txt
>
100 50
150 50
EOF
echo "120	4" > answer.txt
echo "120 4" | gmt select -Lline.txt+d50 > result.txt
diff answer.txt result.txt --strip-trailing-cr > fail
