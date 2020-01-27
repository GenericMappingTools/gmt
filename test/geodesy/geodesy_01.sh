#!/usr/bin/env bash
#

ps=geodesy_01.ps

gmt set PROJ_LENGTH_UNIT inch MAP_VECTOR_SHAPE 0.4 MAP_TICK_LENGTH_PRIMARY 0.075i MAP_FRAME_WIDTH 0.1i MAP_ORIGIN_X 2.5c MAP_ORIGIN_Y 1.3i

#     The example should plot some residual rates of  rotation  in
#     the  Western Transverse Ranges, California.  The wedges will
#     be dark gray, with light gray wedges  to  represent  the  2-
#     sigma uncertainties.

gmt psvelo << EOF  -X2i -Y5i -Jm1.3i -R238.5/242/32.5/35.5 -B2 -BWeSn+tpsvelo \
    -Sw0.4/1.e7 -W0.25p -G60 -E210 -D2 -P -K > $ps
# lon     lat    spin(rad/yr) spin_sigma (rad/yr)
241.4806 34.2073  5.65E-08 1.17E-08
241.6024 34.4468 -4.85E-08 1.85E-08
241.0952 34.4079  4.46E-09 3.07E-08
EOF

# omit the shading
gmt psvelo -J -R -Sw0.4/1.e7 -W0.25p -D2 -O -K << EOF >> $ps
# lon     lat    spin(rad/yr) spin_sigma (rad/yr)
241.2542 34.2581  1.28E-07 1.59E-08
242.0593 34.0773 -6.62E-08 1.74E-08
241.0553 34.5369 -2.38E-07 4.27E-08
241.1993 33.1894 -2.99E-10 7.64E-09
241.1084 34.2565  2.17E-08 3.53E-08
EOF

# hit the beach
gmt pscoast -O -R -J -W0.25p -Di -K >> $ps


#     The  following  should  make  big  green arrows  with blue
#     ellipses,  outlined  in  red. Note that the 39% confidence
#     scaling will give an ellipse which fits inside a rectangle
#     of dimension Esig by Nsig.

#
gmt psvelo -Y-4.5i -R-10/10/-10/10 -Wthin,red \
	-Se0.2/0.39+f12p -B1g1 -BWeSn -Jx0.2i -Ggreen -Eblue -L -N \
	-A1c+p3p+e -O -K << EOF >> $ps
# Long.   Lat.   Evel   Nvel   Esig   Nsig  CorEN SITE
# (deg)  (deg)    (mm/yr)        (mm/yr)
  -10.    0.     5.0    0.0     4.0    6.0  0.500  4x6
   -5.    0.     0.0    5.0     4.0    6.0  0.500  4x6
   -5.    5.    -5.0    0.0     4.0    6.0  0.500  4x6
  -10.    5.     0.0   -5.0     0.0    0.0  0.500  4x6
  -1.     5.     3.0    3.0     1.0    1.0  0.100  3x3
EOF

# simpler colors, labeled with following font
gmt set FONT_ANNOT_PRIMARY Helvetica
gmt psvelo -Se0.2/0.39+f18p -R -J -A0.25c+p0.25p+e -O -Umeca_4 << EOF >> $ps
# Long.   Lat.   Evel   Nvel   Esig   Nsig  CorEN SITE
# (deg)  (deg)    (mm/yr)        (mm/yr)
   0.    -8.     0.0    0.0     4.0    6.0  0.100  4x6
  -8.     5.     3.0    3.0     0.0    0.0  0.200  3x3
   0.     0.     4.0    6.0     4.0    6.0  0.300
  -5.    -5.     6.0    4.0     6.0    4.0  0.400  6x4
   5.     0.    -6.0    4.0     6.0    4.0 -0.300  -6x4
   0.    -5.     6.0   -4.0     6.0    4.0 -0.500  6x-4
EOF
