#!/usr/bin/env bash
gmt begin GMT_velo
	gmt velo <<- EOF -JM12c -R0/4/0/1 -Sw0.915c/5.e7 -W0.5p -Elightred -Glightblue -L0.5p -D2
	0.5 0.493  5.65E-08 1.17E-08
	EOF
	gmt velo -Se0.44c/0.39 -A0.5c+p0.25p+e -Glightblue -Elightred -L0.25p -W2p <<- EOF
	1.25     0.21     3.0    3.0     2.0    1.0  0.300
	EOF
	gmt velo -Sn0.425c -Glightred -Elightred  -W4p,orange <<- EOF
	2.3     0.215     3.0    4.0
	EOF
	gmt velo -Sx0.405c -A0.5c+h0.5 -W2p,red <<- EOF
	3.5     0.5     3.5    -2.5 30
	EOF
	gmt text -F+f7p -D0/-1.2c -N <<- EOF
	0.58 0.5 ROTATIONAL WEDGE
	1.61 0.5 VELOCITY ELLIPSE
	2.5 0.5 ANISOTROPY BAR
	3.5 0.5 STRAIN CROSS
	EOF
gmt end show
