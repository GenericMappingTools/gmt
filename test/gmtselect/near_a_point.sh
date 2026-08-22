#!/usr/bin/env bash
#
# Test script used for problem reported bin gmt-user forum message
# http://gmt.soest.hawaii.edu/boards/1/topics/6027

# Some test data
cat << EOF > t.txt
-110	40	town1
-108	38	town2
-109	43	town3
EOF
echo "-109.9	40.1" > tc.txt
echo "-110	40	town1" > answer.txt
gmt select t.txt -C30k/tc.txt -fg > result.txt
diff answer.txt result.txt --strip-trailing-cr > fail
