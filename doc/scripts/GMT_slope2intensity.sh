#!/usr/bin/env bash
# Illustrate how surface slopes and illumination direction
# interact to yield an intensity for shading purposes
cat << EOF > t.txt
0	0
1	1
1.2	1.03
2	1
3	1.25
4	1.8
5	2.5
6	3.2
6.5	3.3
7	2.7
8	2.6
9	2.65
10	2.3
11	2
12	1.8
13	1.4
14	1.38
15	1.37
EOF
gmt sample1d -I0.1 t.txt -Fa > t2.txt
cat << EOF >> t2.txt
15	1.371
15	0
0	0
EOF
gmt begin GMT_slope2intensity ps
	gmt set GMT_THEME cookbook
	gmt plot -R2/15/1.1/8.4 -Jx0.44444i -B0 t2.txt -Glightgreen -W0.25p
	gmt plot -Sv14p+e+jb+h0.5 -W1p,orange -Gorange -N <<- EOF
	15	4	210	0.75i
	14.5	4.866	210	0.75i
	14	5.732	210	0.75i
	13.5	6.598	210	0.75i
	13	7.4641	210	0.75i
	12.5	8.330	210	0.75i
	EOF
	gmt plot -W2p <<- EOF
	> left
	4.5	2.15
	5.5	2.85
	> right
	9.5	2.5
	10.5	2.15
	> v1 -W0.5p
	5	4
	5	1.5
	> v2 -W0.5p
	10	4
	10	1.5
	> sun -W0.25p,-
	15	4
	10	12.66025
	> line to sun -W0.25p,-
	5	2.45
	15	8.2235
	> line to sun -W0.25p,-
	10	2.325
	15	5.2118
	EOF
	gmt plot -Sv14p+e+h0.5 -W1p -Gblack -N <<- EOF
	5	2.45	124	0.75i
	5	2.45	30	0.75i
	10	2.325	70	0.75i
	10	2.325	30	0.75i
	EOF
	gmt plot -Sw0.5i -W0.25p -N <<- EOF
	5	2.45	30	124
	10	2.325	30	70
	EOF
	echo 	13.7	6.5	-60	14p,Helvetica,orange	CB	LIGHT SOURCE | gmt text -F+a+f+j -Gwhite -C0
	gmt text -F+a+f+j <<- EOF 
	4.1		3.9	0	16p,Times-Roman	RB	<math>\\hat{\\mathbf{n}}_1</math>
	10.7	4.1	0	16p,Times-Roman	RB	<math>\\hat{\\mathbf{n}}_2</math>
	11.5	2.7	0	16p,Times-Roman	LB	<math>\\hat{\\mathbf{s}}</math>
	6.8		3.0	0	16p,Times-Roman	LB	<math>\\hat{\\mathbf{s}}</math>
	EOF
gmt end show
rm -f t.txt t2.txt
