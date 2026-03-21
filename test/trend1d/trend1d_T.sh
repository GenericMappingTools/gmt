#!/usr/bin/env bash
# Testing gmt trend1d -T option for equidistant output
#
# Test written by Claude

# Test 1: Linear fit y = 2x, evaluate at specified points
cat << EOF > input.txt
1	2
2	4
3	6
4	8
5	10
EOF

# Expected output: model evaluated at x = 0, 1, 2, 3, 4, 5, 6
cat << EOF > expected1.txt
0	0
1	2
2	4
3	6
4	8
5	10
6	12
EOF

gmt trend1d input.txt -Fxm -Np1 -T0/6/1 > result1.txt
diff -q result1.txt expected1.txt --strip-trailing-cr || exit 1

# Test 2: Linear fit with only increment (uses data range 1-5)
cat << EOF > expected2.txt
1	2
2	4
3	6
4	8
5	10
EOF

gmt trend1d input.txt -Fxm -Np1 -T1 > result2.txt
diff -q result2.txt expected2.txt --strip-trailing-cr || exit 1

# Test 3: Quadratic fit y = x^2
cat << EOF > input_quad.txt
0	0
1	1
2	4
3	9
4	16
EOF

cat << EOF > expected3.txt
0	0
0.5	0.25
1	1
1.5	2.25
2	4
2.5	6.25
3	9
3.5	12.25
4	16
EOF

gmt trend1d input_quad.txt -Fxm -Np2 -T0/4/0.5 > result3.txt
diff -q result3.txt expected3.txt --strip-trailing-cr || exit 1

# Test 4: Fourier fit y = cos(pi*x/2), period = 4
cat << EOF > input_fourier.txt
0	1
1	0
2	-1
3	0
4	1
EOF

# Expected: cos(pi*x/2) evaluated at x = 0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4
# Use numerical comparison due to floating-point precision
gmt trend1d input_fourier.txt -Fxm -Nc1+o0+l4 -T0/4/0.5 > result4.txt
awk 'BEGIN {pi=3.14159265358979; tol=1e-10}
{
  expected = cos(pi*$1/2)
  diff = ($2 - expected)
  if (diff < 0) diff = -diff
  if (diff > tol) {print "FAIL: x="$1" got "$2" expected "expected; exit 1}
}' result4.txt || exit 1

# Test 5: Validation - should fail with -Fy and -T
if gmt trend1d input.txt -Fxy -Np1 -T1/5/1 2>/dev/null; then
	echo "FAIL: -Fy with -T should have failed"
	exit 1
fi

# Test 6: Validation - should fail with -Fr and -T
if gmt trend1d input.txt -Fxr -Np1 -T1/5/1 2>/dev/null; then
	echo "FAIL: -Fr with -T should have failed"
	exit 1
fi

# Clean up
rm -f input.txt input_quad.txt input_fourier.txt expected*.txt result*.txt

echo "All trend1d -T tests passed"
