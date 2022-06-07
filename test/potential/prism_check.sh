#!/usr/bin/env bash
# Check that the calculation for faa, vgg, and going over a prism is correct
# Results validated by hand with separate Matlab code as well as talwani3d
# We test both Cartesian (in km) and geographic (degrees)

cat <<- EOF > answer.txt
10.000000	10.000000	7000.000000	5.186595
10.000000	10.000000	7000.000000	-7.577306
10.00	10.00	7000.00	0.23
0.100000	0.100000	7000.000000	4.862104
0.100000	0.100000	7000.000000	-7.670899
0.10	0.10	7000.00	0.26
EOF
# Prism centered at (0,0) ranging from z = 0-5000 m
echo "0 0 0 5000" > p.txt
# Cartesian observation point at (10, 10) km at 7000 m elevation
echo " 10 10 7000" > c.txt
# Geographic observation point at lon/lat (0.1, 0.1) at 7000 m elevation
echo "0.1 0.1 7000" > g.txt
# Cartesian prism has dimensions 10x10 km
gmt gravprisms p.txt -E10 -Nc.txt -Mh -D1000 -Ff --FORMAT_FLOAT_OUT=%.6lf > result.txt
gmt gravprisms p.txt -E10 -Nc.txt -Mh -D1000 -Fv --FORMAT_FLOAT_OUT=%.6lf >> result.txt
gmt gravprisms p.txt -E10 -Nc.txt -Mh -D1000 -Fn --FORMAT_FLOAT_OUT=%.2lf >> result.txt
# Geographic prism has dimension 0.1 degrees by 0.1 degrees in longitude/latitude
gmt gravprisms p.txt -E0.1 -Ng.txt -fg -D1000 -Ff --FORMAT_FLOAT_OUT=%.6lf >> result.txt
gmt gravprisms p.txt -E0.1 -Ng.txt -fg -D1000 -Fv --FORMAT_FLOAT_OUT=%.6lf >> result.txt
gmt gravprisms p.txt -E0.1 -Ng.txt -fg -D1000 -Fn --FORMAT_FLOAT_OUT=%.2lf >> result.txt
diff result.txt answer.txt --strip-trailing-cr > fail
## talwani3d solutions:
#cat <<- EOF > square.txt
#-5	-5
#5	-5
#5	5
#-5	5
#-5	-5
#EOF
#gmt math -T0/5000/10 -o0 T = z.txt
#rm -f body.txt
#while read z; do
#	echo "> $z 1000" >> body.txt
#	cat square.txt >> body.txt
#done < z.txt
#gmt talwani3d body.txt -Fg -Mh -Z7 -Nc.txt
#gmt talwani3d body.txt -Fv -Mh -Z7 -Nc.txt
#gmt talwani3d body.txt -Fn -Mh -Z7 -Nc.txt
