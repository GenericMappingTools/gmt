#!/bin/csh -xe
#	$Id$

\rm -f .gmtdefaults4
gmtset MEASURE_UNIT inch TICK_LENGTH 0.075i FRAME_WIDTH 0.1i \
    X_ORIGIN 2.5c Y_ORIGIN 1.3i PAPER_MEDIA  A4 DEGREE_FORMAT 3

set plotfile = meca_4.ps 

#     The example should plot some residual rates of  rotation  in
#     the  Western Transverse Ranges, California.  The wedges will
#     be dark gray, with light gray wedges  to  represent  the  2-
#     sigma uncertainties.
 
psvelo << EOF  -X2i -Y5i -Jm1.3i -R238.5/242/32.5/35.5 -B2/2/:.psvelo:WeSn \
    -Sw0.4/1.e7 -W0.25p -G60 -E210 -h1 -D2 -P -K > $plotfile    
 lon     lat    spin(rad/yr) spin_sigma (rad/yr)
241.4806 34.2073  5.65E-08 1.17E-08
241.6024 34.4468 -4.85E-08 1.85E-08
241.0952 34.4079  4.46E-09 3.07E-08
EOF

# omit the shading
psvelo -Jm1.3i -R238.5/242/32.5/35.5 -Sw0.4/1.e7 \
    -W0.25p -h1 -D2 -P -O -K << EOF >> $plotfile    
 lon     lat    spin(rad/yr) spin_sigma (rad/yr)
241.2542 34.2581  1.28E-07 1.59E-08
242.0593 34.0773 -6.62E-08 1.74E-08
241.0553 34.5369 -2.38E-07 4.27E-08
241.1993 33.1894 -2.99E-10 7.64E-09
241.1084 34.2565  2.17E-08 3.53E-08
EOF
 
# hit the beach
pscoast -P -O -R238.5/242/32.5/35.5 -Jm1.3i -W0.25p -Di -K >> $plotfile    


#     The  following  should  make  big  red  arrows  with   green
#     ellipses,  outlined  in  red.   Note that the 39% confidence
#     scaling will give an ellipse which fits inside  a  rectangle
#     of dimension Esig by Nsig.

gmtset DEGREE_FORMAT 0
 
#
psvelo << EOF -Y-4.5i -h2 -R-10/10/-10/10 -W5/255/0/0 \
 -Se0.2/0.39/12 -B1g1/WeSn -Jx0.2i/0.2i -G0/255/0 -E0/0/255 -L -N \
-A0.1i/0.76c/0.3i -P -V -O -K >> $plotfile    
  Long.   Lat.   Evel   Nvel   Esig   Nsig  CorEN SITE
  (deg)  (deg)    (mm/yr)        (mm/yr)
  -10.    0.     5.0    0.0     4.0    6.0  0.500  4x6
   -5.    0.     0.0    5.0     4.0    6.0  0.500  4x6
   -5.    5.    -5.0    0.0     4.0    6.0  0.500  4x6
  -10.    5.     0.0   -5.0     0.0    0.0  0.500  4x6
  -1.     5.     3.0    3.0     1.0    1.0  0.100  3x3
EOF

# simpler colors, labeled with following font
gmtset ANOT_FONT  Helvetica
psvelo -h2 -Se0.2/0.39/18 -R-10/10/-10/10 -Jx0.2i/0.2i \
    -P -V -O -Umeca_4 << EOF >> $plotfile    
  Long.   Lat.   Evel   Nvel   Esig   Nsig  CorEN SITE
  (deg)  (deg)    (mm/yr)        (mm/yr)
   0.    -8.     0.0    0.0     4.0    6.0  0.100  4x6
  -8.     5.     3.0    3.0     0.0    0.0  0.200  3x3
   0.     0.     4.0    6.0     4.0    6.0  0.300
  -5.    -5.     6.0    4.0     6.0    4.0  0.400  6x4
   5.     0.    -6.0    4.0     6.0    4.0 -0.300  -6x4
   0.    -5.     6.0   -4.0     6.0    4.0 -0.500  6x-4
EOF
\rm -f .gmtdefaults4
