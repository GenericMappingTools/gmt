#!/bin/bash
#	$Id$

. ./functions.sh
header "Test psmeca for plotting focal mechanisms (2b)"

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

plots () {
y_offset=-2.5i
for a in $1 $2 $3 ; do
    pscoupe -R0/250/0/100 -JX1.5i/-1.5i -Ba100f10/a50f10WesN \
        -Q -L -Sc0.4 -Ab$4/$5/$a/250/90/$6/0/100f -Ggrey -a0.1i/cc $7 $8 \
        -Y$y_offset -X$x_offset -O -K << EOF
# lon   lat  dep str dip rake str dip rake m ex nx ny 
129.5 10.5  10  0   90   0  90   90 180  1 24  0  0 10km
128.5 10.5  40  0   45  90 180   45  90  1 24  0  0 40km
EOF
    pstext -R -J -F+f18p,Helvetica-Bold+jBR -O -K <<< "240 90 $a"
    y_offset=0i
    x_offset=2.5i
done
x_offset=-5i
}

x_offset=0i

plots   0  40  80 128 10.0 200    >> $ps
plots 120 160 200 128 11.0 400 -N >> $ps
plots 240 280 320 130 10.5 200 -N >> $ps

pstext -X-5i -R0/10/0/15 -Jx1i -F+jBL+fHelvetica-Bold+f -O << EOF >> $ps
3 8.5 24 Variation of azimuth
3 8.0 20 vertical cross-section
EOF

pscmp
