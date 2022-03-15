#!/usr/bin/env bash
# Check that the calculation for gravity over a prism is correct
# Results validated by hand with separate Matlab code
# We test both Cartesian (in km) and geographic (degrees)

cat <<- EOF > answer.txt
10.000000	10.000000	7000.000000	5.186362
0.100000	0.100000	7000.000000	4.861885
EOF
# Prism centered at (0,0) ranging from z = 0-5000 m
echo "0 0 0 5000" > p.txt
# Cartesian observation point at (10, 10) km at 7000 m elevation
echo " 10 10 7000" > c.txt
# Geopgraphic observation point at lon/lat (0.1, 0.1) at 7000 m elevation
echo "0.1 0.1 7000" > g.txt
# Cartesian prism has dimensions 10x10 km
gmt gravprisms p.txt -E10 -Nc.txt -Mh -D1000 --FORMAT_FLOAT_OUT=%.6lf > result.txt
# Geographic prism has dimension 0.1 degrees by 0.1 degrees in longitude/latitude
gmt gravprisms p.txt -E0.1 -Ng.txt -fg -D1000 --FORMAT_FLOAT_OUT=%.6lf >> result.txt
diff result.txt answer.txt --strip-trailing-cr > fail
