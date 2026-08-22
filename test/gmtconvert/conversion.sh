#!/usr/bin/env bash
# Test gmt convert with both -o and -b
echo 1 2 3 4 5 > t.txt
gmt convert t.txt -o0,1,4 -bo3d > d.b
gmt convert t.txt -o0,1,4 -bo3f > s.b
D=$(ls -l d.b | awk '{print $5}')
S=$(ls -l d.b | awk '{print $5}')
touch fail
if [ $D -ne $S ]; then
	echo "db and sb differ in size" >> fail
fi
gmt convert d.b -bi3d > d.txt
gmt convert s.b -bi3f > s.txt
diff d.txt s.txt --strip-trailing-cr >> fail
