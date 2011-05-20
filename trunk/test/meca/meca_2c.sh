#!/bin/bash
#	$Id: meca_2c.sh,v 1.3 2011-05-20 19:16:02 remko Exp $

. ../functions.sh
header "Test psmeca for plotting focal mechanisms (2c)"

ps=meca_2c.ps

gmtset PROJ_LENGTH_UNIT inch MAP_TICK_LENGTH 0.075i MAP_FRAME_WIDTH 0.1i MAP_ORIGIN_X 2.5c MAP_ORIGIN_Y 1.3i

psmeca -R128/130/10/11.1 -JX2i -a0.2/cc -Sc0.4 -B1 -Y8.5 -K << EOF > $ps
# lon   lat  dep str dip rake str dip rake m ex nx ny 
129.5  10.5  0  0   90   0  90   90 180  1 24  0  0 10km
128.5  10.5  0  0   45  90 180   45  90  1 24  0  0 40km
EOF

y_offset=-2.5
x_offset=0
for a in 0 40 80 ; do
    pscoupe -R0/350/0/100 -JX1.5i -Ba100f10/a50f10WesN \
        -L -Sc0.4 -Ab128/10/$a/250/90/200/0/100f -G200 -a0.1i/cc \
        -Y$y_offset -X$x_offset -O -K << EOF >> $ps
# lon   lat  dep str dip rake str dip rake m ex nx ny 
129.5 10.5  10  0   90   0  90   90 180  1 24  0  0 10-$a
128.5 10.5  40  0   45  90 180   45  90  1 24  0  0 40-$a
EOF
    pstext -R -J -F+f18p,9+jBR -O -K >> $ps <<< "300 90 $a"
    y_offset=0
    x_offset=2.5
done
y_offset=-2.5
x_offset=-5
for a in 120 160 200 ; do
    pscoupe -R -J -Ba100f10/a50f10WesN -Ewhite \
        -N -L -Sc0.4 -Ab128/11/$a/250/90/400/0/100f -G200 -a0.1i/cc \
        -Y$y_offset -X$x_offset -O -K << EOF >> $ps
# lon   lat  dep str dip rake str dip rake m ex nx ny
129.5 10.5  10  0   90   0  90   90 180  1 24  0  0 10-$a
128.5 10.5  40  0   45  90 180   45  90  1 24  0  0 40-$a
EOF
    pstext -R -J -F+f18p,9+jBR -O -K >> $ps <<< "300 90 $a"
    y_offset=0
    x_offset=2.5
done
y_offset=-2.5
x_offset=-5
for a in 240 280 320 ; do
    pscoupe -R -J -Ba100f10/a50f10WesN -Ewhite \
        -N -L -Sc0.4 -Ab130/10.5/$a/250/90/200/0/100f -G200 -a0.1i/cc \
        -Y$y_offset -X$x_offset -O -K << EOF >> $ps
# lon   lat  dep str dip rake str dip rake m ex nx ny
129.5 10.5  10  0   90   0  90   90 180  1 24  0  0 10-$a
128.5 10.5  40  0   45  90 180   45  90  1 24  0  0 40-$a
EOF
    pstext -R -J -F+f18p,9+jBR -O -K >> $ps <<< "300 90 $a"
    y_offset=0
    x_offset=2.5
done
pstext -X-5i -R0/10/0/15 -F+jBL+fHelvetica-Bold+f -Jx1i -O << EOF >> $ps
3 8.5 24 Variation of azimuth
3 8.0 20 vertical cross-section
EOF

rm -f Ab*

pscmp
