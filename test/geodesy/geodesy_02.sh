#!/usr/bin/env bash
#
ps=geodesy_02.ps

#     The example should plot some residual rates of rotation  in
#     the  Western Transverse Ranges, California.  The wedges will
#     be dark gray, with light gray wedges  to  represent  the  2-
#     sigma uncertainties. [K. Feigl, 2015-11-08]

gmt psvelo << EOF -Xc -JM15c -R241/242/34/35 -B0.5 -BWeSn+tpsvelo \
    -Sw3c/1.e7 -W1p -G60 -E210 -D2 -P -K > $ps   
# lon     lat    spin(rad/yr) spin_sigma (rad/yr)
241.2 34.2  5.65E-08 1.17E-08
241.2 34.5 -4.85E-08 1.85E-08
241.2 34.7  4.46E-09 3.07E-08
EOF

# omit the shading
gmt psvelo -J -R -Sw3c/1.e7 -W1p -D2 -O -K << EOF >> $ps   
# lon     lat    spin(rad/yr) spin_sigma (rad/yr)
# lon     lat    spin(rad/yr) spin_sigma (rad/yr)
241.7 34.2  5.65E-08 1.17E-08
241.7 34.5 -4.85E-08 1.85E-08
241.7 34.7  4.46E-09 3.07E-08
EOF

# hit the beach
gmt pscoast -O -R -J -W1p -Di >> $ps   
