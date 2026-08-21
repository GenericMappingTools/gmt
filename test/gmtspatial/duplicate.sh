#!/usr/bin/env bash
# Test gmtspatial duplicate function after fixing the dumb edit that
# caused https://forum.generic-mapping-tools.org/t/duplicate-lines-or-line-segments-using-gmt-spatial/2245
cat << EOF > answer.txt
- : Input line [ segment 0 ] is an approximate-subset duplicate of a line [ segment 1 ] in the same data set, with d = 0.0199000010622 c = 0.00111432 s = 2.319
Y : Input line [ segment 2 ] is an exact duplicate of a line [ segment 3 ] in the same data set, with d = 0 c = 0 s = 0
EOF
gmt spatial andes_test.txt -fg -D+d10k+C0.2+p -V2 > result.txt
diff -q --strip-trailing-cr answer.txt result.txt > fail
