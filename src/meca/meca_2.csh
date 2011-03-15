#!/bin/csh -f
#	$Id: meca_2.csh,v 1.5 2011-03-15 02:06:37 guru Exp $

\rm -f .gmtdefaults4
gmtset MEASURE_UNIT inch TICK_LENGTH 0.075i FRAME_WIDTH 0.1i \
    X_ORIGIN 2.5c Y_ORIGIN 1.3i DEGREE_FORMAT 3

set plotfile = meca_2.ps

set c_frame = -R0/200/0/100
set c_proj = (-JX1.5i/-1.5i -P)

#Plotting 2 mechanisms on map
psmeca -P -R128/130/10/11.1 \
     -h1 -a0.1i/cc -JX2i -Sc0.4i -B1 -Y8.5i -K << EOF >! $plotfile
lon   lat  dep str dip rake str dip rake m ex nx ny 
129.5  10.5 10  0   90   0  90   90 180  1 24  0  0 10km
128.5  10.5 40  0   45  90 180   45  90  1 24  0  0 40km
EOF
(echo 128 11; echo 130 11) | psxy -R -JX -K -O -W0.25p/255/0/0 >> $plotfile
pstext -R -N -JX -K -O << EOF >> $plotfile
128 11 14 0 1 5 P1
130 11 14 0 1 7 P2
EOF

#Represent cross-sections between points P1(128E11N) and P2(130E/11N)
# on a plane the dip of which varies
# from quasi horizontal to vertical.
# y dimension is counted along steepest descent on the plane
# so the values of depth are only in the vertical cross-section.
#Additional files are created (the names are derived from cross-sectioon
# attributes) :
# - the first one contains new coordinates in km
# origin is first point of cross-section definition
# x is counted along strike of plane
# y is counted along steepest descent
# third column is the depth
# In case of mechanism, half-sphere behind the plane is represented.
# - the second one (...map) contains events that are used in 
# the cross-section (extracted from input file)

# Variation of dip for cross-section plane 
# (WE azimuth, between points (128E,11N) and (130E,11N))) 
@ d = 10
set y_offset = -2.5
set x_offset = 0
while ($d <= 30)
    pscoupe $c_frame $c_proj \
        -Ba100f10/a50f10WesN \
        -L -h1 -Sc0.4i -Aa128/11/130/11/$d/60/0/100f -G200 -V -a0.1i/cc \
        -Y$y_offset -X$x_offset -O -K << EOF >> $plotfile
lon   lat  dep str dip rake str dip rake m ex nx ny 
129.5 10.5  10  0   90   0  90   90 180  1 24  0  0 10-$d
128.5 10.5  40  0   45  90 180   45  90  1 24  0  0 40-$d
EOF
    pstext $c_frame $c_proj -O -K << EOF >> $plotfile
10 5 18 0 1 9 $d
EOF
    @ d = $d + 10
    set y_offset = 0
    set x_offset = 2.5
end
set y_offset = -2.5
set x_offset = -5
while ($d <= 60)
    pscoupe $c_frame $c_proj \
        -Ba100f10/a50f10WesN \
        -L -h1 -Sc0.4 -Aa128/11/130/11/$d/60/0/100f -G200 -V -a0.1i/cc \
        -Y$y_offset -X$x_offset -O -K << EOF >> $plotfile
lon   lat  dep str dip rake str dip rake m ex nx ny
129.5 10.5  10  0   90   0  90   90 180  1 24  0  0 10-$d
128.5 10.5  40  0   45  90 180   45  90  1 24  0  0 40-$d
EOF
    pstext $c_frame $c_proj -O -K << EOF >> $plotfile
10 5 18 0 1 9 $d
EOF
    @ d = $d + 10
    set y_offset = 0
    set x_offset = 2.5
end
set y_offset = -2.5
set x_offset = -5
while ($d <= 90)
    pscoupe $c_frame $c_proj \
        -Ba100f10/a50f10WesN \
        -L -h1 -Sc0.4 -Aa128/11/130/11/$d/60/0/100f -G200 -V -a0.1i/cc \
        -Y$y_offset -X$x_offset -O -K << EOF >> $plotfile
lon   lat  dep str dip rake str dip rake m ex nx ny
129.5 10.5  10  0   90   0  90   90 180  1 24  0  0 10-$d
128.5 10.5  40  0   45  90 180   45  90  1 24  0  0 40-$d
EOF
    pstext $c_frame $c_proj -O -K << EOF >> $plotfile
10 5 18 0 1 9 $d
EOF
    @ d = $d + 10
    set y_offset = 0
    set x_offset = 2.5
end
pstext -X-5 -R0/10/0/15 -Jx1 -O << EOF >> $plotfile
3 8.5 24 0 1 1 Variation of dip
3 8.0 20 0 1 1 W-E cross-section
EOF

# Variation of azimuth for vertical cross-section plane
set c_frame = -R0/350/0/100
set c_proj = (-JX1.5/-1.5 -P)

psmeca -P -R128/130/10/11.1 \
     -h1 -a0.2/cc -JX2 -Sc0.4 -B1 -Y8.5 -K << EOF >> $plotfile
lon   lat  dep str dip rake str dip rake m ex nx ny 
129.5  10.5  0  0   90   0  90   90 180  1 24  0  0 10km
128.5  10.5  0  0   45  90 180   45  90  1 24  0  0 40km
EOF

@ a = 0
set y_offset = -2.5
set x_offset = 0
while ($a <= 80)
    pscoupe $c_frame $c_proj \
        -Ba100f10/a50f10WesN \
        -L -h1 -Sc0.4 -Ab128/10/$a/250/90/200/0/100f -G200 -V -a0.1i/cc \
        -Y$y_offset -X$x_offset -O -K << EOF >> $plotfile
lon   lat  dep str dip rake str dip rake m ex nx ny 
129.5 10.5  10  0   90   0  90   90 180  1 24  0  0 10-$a
128.5 10.5  40  0   45  90 180   45  90  1 24  0  0 40-$a
EOF
    pstext $c_frame $c_proj -O -K << EOF >> $plotfile
300 90 18 0 9 3 $a
EOF
    @ a = $a + 40
    set y_offset = 0
    set x_offset = 2.5
end
set y_offset = -2.5
set x_offset = -5
while ($a <= 200)
    pscoupe $c_frame $c_proj \
        -Ba100f10/a50f10WesN -E255/255/255 \
        -N -L -h1 -Sc0.4 -Ab128/11/$a/250/90/400/0/100f -G200 -V -a0.1i/cc \
        -Y$y_offset -X$x_offset -O -K << EOF >> $plotfile
lon   lat  dep str dip rake str dip rake m ex nx ny
129.5 10.5  10  0   90   0  90   90 180  1 24  0  0 10-$a
128.5 10.5  40  0   45  90 180   45  90  1 24  0  0 40-$a
EOF
    pstext $c_frame $c_proj -O -K << EOF >> $plotfile
300 90 18 0 9 3 $a
EOF
    @ a = $a + 40
    set y_offset = 0
    set x_offset = 2.5
end
set y_offset = -2.5
set x_offset = -5
while ($a <= 320)
    pscoupe $c_frame $c_proj \
        -Ba100f10/a50f10WesN -E255/255/255 \
        -N -L -h1 -Sc0.4 -Ab130/10.5/$a/250/90/200/0/100f -G200 -V -a0.1i/cc \
        -Y$y_offset -X$x_offset -O -K << EOF >> $plotfile
lon   lat  dep str dip rake str dip rake m ex nx ny
129.5 10.5  10  0   90   0  90   90 180  1 24  0  0 10-$a
128.5 10.5  40  0   45  90 180   45  90  1 24  0  0 40-$a
EOF
    pstext $c_frame $c_proj -O -K << EOF >> $plotfile
300 90 18 0 9 3 $a
EOF
    @ a = $a + 40
    set y_offset = 0
    set x_offset = 2.5
end
pstext -X-5 -R0/10/0/15 -Jx1 -O << EOF >> $plotfile
3 8.5 24 0 1 1 Variation of azimuth
3 8.0 20 0 1 1 vertical cross-section
EOF
\rm -f Aa* Ab* .gmtdefaults4
