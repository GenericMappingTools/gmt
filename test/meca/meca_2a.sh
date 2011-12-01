#!/bin/bash
#	$Id$

. ../functions.sh
header "Test psmeca for plotting focal mechanisms (2a)"

ps=meca_2a.ps

gmtset PROJ_LENGTH_UNIT inch MAP_TICK_LENGTH_PRIMARY 0.075i MAP_FRAME_WIDTH 0.1i MAP_ORIGIN_X 2.5c MAP_ORIGIN_Y 1.3i

# Plotting 2 mechanisms on map
psmeca -R128/130/10/11.1 -JX2i -a0.1i/cc -Sc0.4i -B1 -Y8.5i -P -K << EOF > $ps
# lon   lat  dep str dip rake str dip rake m ex nx ny 
129.5  10.5 10  0   90   0  90   90 180  1 24  0  0 10km
128.5  10.5 40  0   45  90 180   45  90  1 24  0  0 40km
EOF
(echo 128 11; echo 130 11) | psxy -R -J -K -O -W0.25p,red >> $ps
pstext -R -J -N -F+f14p,Helvetica-Bold+j -K -O << EOF >> $ps
128 11 ML P1
130 11 MR P2
EOF

# Represent cross-sections between points P1(128E11N) and P2(130E/11N)
# on a plane the dip of which varies
# from quasi horizontal to vertical.
# y dimension is counted along steepest descent on the plane
# so the values of depth are only in the vertical cross-section.

# Variation of dip for cross-section plane 
# (WE azimuth, between points (128E,11N) and (130E,11N))) 

plots () {
y_offset=-2.5i
for d in $1 $2 $3 ; do
    pscoupe -R0/200/0/100 -JX1.5i/-1.5i -Ba100f10/a50f10WesN \
        -Q -L -Sc0.4 -Aa128/11/130/11/$d/60/0/100f -Ggrey -a0.1i/cc $4 $5 \
        -Y$y_offset -X$x_offset -O -K << EOF
# lon   lat  dep str dip rake str dip rake m ex nx ny 
129.5 10.5  10  0   90   0  90   90 180  1 24  0  0 10km
128.5 10.5  40  0   45  90 180   45  90  1 24  0  0 40km
EOF
    pstext -R -J -F+f18p,Helvetica-Bold+jBL -O -K <<< "10 15 $d"
    y_offset=0i
    x_offset=2.5i
done
x_offset=-5i
}

x_offset=0i

plots 10 20 30 >> $ps
plots 40 50 60 >> $ps
plots 70 80 90 -N >> $ps

pstext -X-5i -R0/10/0/15 -Jx1i -F+jBL+fHelvetica-Bold+f -O << EOF >> $ps
3 8.5 24 Variation of dip
3 8.0 20 W-E cross-section
EOF

pscmp
