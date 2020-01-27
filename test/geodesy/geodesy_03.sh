#!/usr/bin/env bash

ps=geodesy_03.ps

#     The  following  should  make  big  green arrows  with blue
#     ellipses,  outlined  in  red. Note that the 39% confidence
#     scaling will give an ellipse which fits inside a rectangle
#     of dimension Esig by Nsig. [K. Feigl, 2015-11/08]

gmt psvelo -Xc -R-3/6/-3/7 -Wthin,red -Se1.5c/0.39+f12p -Bpxa1g1 \
	-Bpya1g1 -BWeSn+t"E = 3 @~\261@~ 1; N = 4 @~\261@~ 2" -Jx1.5c -Ggreen -Eblue -L -N \
	-A1c+p3p+e -P -h2 << EOF > $ps
# Long.   Lat.   Evel   Nvel   Esig   Nsig  CorEN SITE
# (deg)  (deg)    (mm/yr)        (mm/yr)
    0.    0.     3.0    0.0     1.    2.0  -0.5  A
    3.    0.     0.0    4.0     1.    2.0  +0.5  B
    3.    4.    -3.0    0.0     1.    2.0   0.0  C
    0.    4.     0.0   -4.0     1.    2.0   0.0  D
    0.    0.     3.0    0.0     0.    0.    0.0  A
EOF
