#!/bin/bash
#	$Id$
#
# Resampling of Cartesian and geographic tracks

# Test the various -A modes for resampling (x,y) or (lon.lat) tracks
ps=sample.ps
RJ="-R-10/50/67/77 -JX6i"
# Pick a few points; this is our track either as Cartesian or degrees
cat << EOF > track.txt
-5	74
43	76
42	73
44	73
38	68
EOF
#Cartesian resampling with small spacing
gmt psxy track.txt $RJ -P -Sc0.2c -Ggreen -W0.25p -Baf -BWSne -K > $ps
gmt psxy track.txt $RJ -O -K -W0.25p >> $ps
# equidistant sampling as is
gmt sample1d track.txt -I2c -Ar | gmt psxy $RJ -O -K -Sc0.1c -Gcyan >> $ps
# equidistant sampling adjusted
gmt sample1d track.txt -I2c -AR | gmt psxy $RJ -O -K -Sc0.1c -Gblack >> $ps
# -Am sampling
gmt sample1d track.txt -I0.1c -Am | gmt psxy $RJ -O -K -W0.25p,blue,- >> $ps
# -Ap sampling
gmt sample1d track.txt -I0.1c -Ap | gmt psxy $RJ -O -K -W0.25p,orange,- >> $ps

#Geographic resampling with small spacing
RJ="-R-10/50/67/77 -JM6i"
gmt psxy track.txt $RJ -O -K -Sc0.2c -Ggreen -W0.25p -Baf -BWSne -Y6.5i >> $ps
gmt psxy track.txt $RJ -O -K -W0.25p >> $ps
# Loxodrome sampling
gmt sample1d track.txt -I100k -AR+l | gmt psxy $RJ -O -K -W0.25p,red,- >> $ps
# equidistant sampling as is
gmt sample1d track.txt -I100k -Ar | gmt psxy $RJ -O -K -Sc0.1c -Gcyan >> $ps
# equidistant sampling adjusted
gmt sample1d track.txt -I100k -AR | gmt psxy $RJ -O -K -Sc0.1c -Gblack >> $ps
# -Am sampling
gmt sample1d track.txt -I10k -Am | gmt psxy $RJ -O -K -W0.25p,blue,- >> $ps
# -Ap sampling
gmt sample1d track.txt -I10k -Ap | gmt psxy $RJ -O -K -W0.25p,orange,- >> $ps
# Geographic downsizing a dense line with coarse spacing
gmt project -C20/73 -E35/68 -G30 -Q > tmp.txt
gmt psxy $RJ -O -K tmp.txt -Sc0.1c -Ggreen >> $ps
gmt sample1d tmp.txt -I100k -AR | gmt psxy $RJ -O -K -Sc0.2c -Gblack >> $ps
gmt pstext -R0/6/o/4 -Jx1i -O -K -F+jLB+f14p,Courier << EOF >> $ps
0.2 0.2 BLACK LINE:  psxy default
0.2 0.4 RED LINE:    Loxodrome mode
0.2 0.6 BLUE LINE:   -Am mode
0.2 0.8 ORANGE LINE: -Ap mode
0.2 1.0 GREEN PTS:   Input
0.2 1.2 BLACK PTS:   -AR mode
0.2 1.4 CYAN PTS:    -Ar mode
EOF
gmt psxy $RJ -O -T >> $ps
