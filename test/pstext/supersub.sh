#!/bin/bash
# Test placement of sub, super-, and both with pstext, including
# font changes, colors, and underline in sub/super scripts.
ps=supersub.ps
cat << EOF > t.txt
0	-3	The depth to Z@+300\312@+@-  p@- is used
0	-2	The depth to Z@-p@-@+300\312@+ is used
0	-1	The range is W \234 @-75@-@+135@+ km
0	0	The stress @~s@~@-xx@-@+@%6%exact@%%@+ = 450 MPa
0	1	The stress @~s@~@-xx@-@+*@+ = 450 MPa
0	2	The stress @~s@~@+*@+@-@_@;red;exact@;;@_@- = 450 MPa
0	3	With @~s@~@+*@+@-yy@- = 50 MPa and x@+2@+ = @~e@~@-0@-.
EOF
gmt pstext -R-3/3/-4/4 -Jx1i -P -Bxafg2 -Byaf -F+f24p t.txt -Xc > $ps
