#!/bin/bash
# Test the gmtvector application

. functions.sh

header "Test gmtvector with basic vector math"

cat << EOF > vec2d.txt
2.5	1
1.5	1
1	1
0.5	1
-1.5	1
EOF
cat << EOF > vec3d.txt
2.5	1	0.5
1.5	1	1.2
1	1	-0.5
0.5	1	2.0
-1.5	1	0.0
EOF
cat << EOF > vecg.txt
0	0
0	90
135	45
-30	-60
EOF

# Use fixed format floats (number of digits of default %lg is unpredictable)
gmtset FORMAT_FLOAT_OUT "%.6e"

# Normalize the 2-D vectors
echo "# gmtvector vec2d.txt -C -N" > result
gmtvector vec2d.txt -C -N >> result
# Normalize the 3-D vectors
echo "# gmtvector vec3d.txt -C -N" >> result
gmtvector vec3d.txt -C -N >> result
# Angles between 2-D vectors and (1,0)
echo "# gmtvector vec3d.txt -C -TD -S1/0" >> result
gmtvector vec3d.txt -C -TD -S1/0 >> result
# Angles between 3-D vectors and (0,0,1)
echo "# gmtvector vec3d.txt -C -TD -S0/0/1" >> result
gmtvector vec3d.txt -C -TD -S0/0/1 >> result
# Angles between geographic vectors and (0,0)
echo "# gmtvector vecg.txt -TD -S0/0 -fg" >> result
gmtvector vecg.txt -TD -S0/0 -fg >> result
# Mean 2-D vectors
echo "# gmtvector vec2d.txt -Am -C" >> result
gmtvector vec2d.txt -Am -C >> result
# Mean 3-D vectors
echo "# gmtvector vec3d.txt -Am -C" >> result
gmtvector vec3d.txt -Am -C >> result
# Mean geo vectors
echo "# gmtvector vecg.txt -Am -fg" >> result
gmtvector vecg.txt -Am -fg >> result
# Convert 2-D Cartesian to polar r/theta
echo "# gmtvector -A1/1 -Ci" >> result
gmtvector -A1/1 -Ci >> result
# Convert 3-D Cartesian to geographic
echo "# gmtvector -A0.61237/0.61237/0.5 -Ci -fg" >> result
gmtvector -A0.612372436/0.612372436/0.5 -Ci -fg >> result
# 3-D cross product
echo "# gmtvector -A1/1/0 -S0/0/1 -Tx -C" >> result
gmtvector -A1/1/0 -S0/0/1 -Tx -C >> result
# Bisector pole 
echo "# gmtvector -A30/30 -S-30/-30 -Tb -fg" >> result
gmtvector -A30/30 -S-30/-30 -Tb -fg >> result
diff -q --strip-trailing-cr result $src/result > fail

passfail vecmath
