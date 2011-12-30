#!/bin/bash
#	$Id$
#

. ../functions.sh
header "Test psmeca for plotting focal mechanisms (4)"

ps=meca_4.ps

gmtset PROJ_LENGTH_UNIT inch MAP_TICK_LENGTH_PRIMARY 0.075i MAP_FRAME_WIDTH 0.1i MAP_ORIGIN_X 2.5c MAP_ORIGIN_Y 1.3i PS_MEDIA A4

#     The example should plot some residual rates of  rotation  in
#     the  Western Transverse Ranges, California.  The wedges will
#     be dark gray, with light gray wedges  to  represent  the  2-
#     sigma uncertainties.
 
psvelo << EOF  -X2i -Y5i -Jm1.3i -R238.5/242/32.5/35.5 -B2/2/:.psvelo:WeSn \
    -Sw0.4/1.e7 -W0.25p -G60 -E210 -D2 -P -K > $ps    
# lon     lat    spin(rad/yr) spin_sigma (rad/yr)
241.4806 34.2073  5.65E-08 1.17E-08
241.6024 34.4468 -4.85E-08 1.85E-08
241.0952 34.4079  4.46E-09 3.07E-08
EOF

# omit the shading
psvelo -Jm1.3i -R238.5/242/32.5/35.5 -Sw0.4/1.e7 -W0.25p -D2 -O -K << EOF >> $ps    
# lon     lat    spin(rad/yr) spin_sigma (rad/yr)
241.2542 34.2581  1.28E-07 1.59E-08
242.0593 34.0773 -6.62E-08 1.74E-08
241.0553 34.5369 -2.38E-07 4.27E-08
241.1993 33.1894 -2.99E-10 7.64E-09
241.1084 34.2565  2.17E-08 3.53E-08
EOF
 
# hit the beach
pscoast -O -R238.5/242/32.5/35.5 -Jm1.3i -W0.25p -Di -K >> $ps    


#     The  following  should  make  big  green arrows  with blue
#     ellipses,  outlined  in  red. Note that the 39% confidence
#     scaling will give an ellipse which fits inside a rectangle
#     of dimension Esig by Nsig.
 
#
psvelo -Y-4.5i -R-10/10/-10/10 -Wthick,red \
	-Se0.2/0.39/12 -B1g1/WeSn -Jx0.2i/0.2i -Ggreen -Eblue -L -N \
	-A0.1i/0.76c/0.3i -O -K << EOF >> $ps    
# Long.   Lat.   Evel   Nvel   Esig   Nsig  CorEN SITE
# (deg)  (deg)    (mm/yr)        (mm/yr)
  -10.    0.     5.0    0.0     4.0    6.0  0.500  4x6
   -5.    0.     0.0    5.0     4.0    6.0  0.500  4x6
   -5.    5.    -5.0    0.0     4.0    6.0  0.500  4x6
  -10.    5.     0.0   -5.0     0.0    0.0  0.500  4x6
  -1.     5.     3.0    3.0     1.0    1.0  0.100  3x3
EOF

# simpler colors, labeled with following font
gmtset FONT_ANNOT_PRIMARY Helvetica
psvelo -Se0.2/0.39/18 -R-10/10/-10/10 -Jx0.2i/0.2i -O -Umeca_4 << EOF >> $ps    
# Long.   Lat.   Evel   Nvel   Esig   Nsig  CorEN SITE
# (deg)  (deg)    (mm/yr)        (mm/yr)
   0.    -8.     0.0    0.0     4.0    6.0  0.100  4x6
  -8.     5.     3.0    3.0     0.0    0.0  0.200  3x3
   0.     0.     4.0    6.0     4.0    6.0  0.300
  -5.    -5.     6.0    4.0     6.0    4.0  0.400  6x4
   5.     0.    -6.0    4.0     6.0    4.0 -0.300  -6x4
   0.    -5.     6.0   -4.0     6.0    4.0 -0.500  6x-4
EOF

pscmp
