#!/usr/bin/env bash
#
# Tests gmt project in generating small and great circles

ps=small.ps

# P is the rotation pole and C is the center.  The great circle through
# P and C defines the zero-meridian.  C is not necessarily 90 degrees
# from P; in our case it is 65.2 degrees from P.  We verify that C lies
# half-way between the red (70) and orange (60) small colatitude circles.
P=85/40
C=15/15
# Test -C -T -G for generating great/small circles about pole P for a certain angular extent (-L)
# The default is a great circle unless a specific /colatitude is appended in -G
# Try plot 110 degree small circle around P given C
gmt project -C$C -T$P -G1/110 -L-20/60 | gmt psxy -Rg -JA40/30/7.5i -P -K -W2p,. -Xc -Yc > $ps
# Try plot great circle around P given C
gmt project -C$C -T$P -G1 -L-20/60 | gmt psxy -Rg -JA40/30/7.5i -O -K -W2p >> $ps
# Try plot 80 degree small circle around P
gmt project -C$C -T$P -G1/80 -L-20/60 | gmt psxy -Rg -JA40/30/7.5i -O -K -W2p,darkgray >> $ps
# Try plot 70 degree small circle around P
gmt project -C$C -T$P -G1/70 -L-20/60 | gmt psxy -Rg -JA40/30/7.5i -O -K -W2p,red >> $ps
# Try plot 60 degree small circle around P
gmt project -C$C -T$P -G1/60 -L-20/60 | gmt psxy -Rg -JA40/30/7.5i -O -K -W2p,orange >> $ps
# Try plot 50 degree small circle  around P
gmt project -C$C -T$P -G1/50 -L-20/60 | gmt psxy -Rg -JA40/30/7.5i -O -K -W2p,yellow >> $ps
# Try plot 40 degree small circle around P
gmt project -C$C -T$P -G1/40 -L-20/60 | gmt psxy -Rg -JA40/30/7.5i -O -K -W2p,green >> $ps
# Try plot 30 degree small circle around P
gmt project -C$C -T$P -G1/30 -L-20/60 | gmt psxy -Rg -JA40/30/7.5i -O -K -W2p,cyan >> $ps
# Try plot 20 degree small circle around P
gmt project -C$C -T$P -G1/20 -L-20/60 | gmt psxy -Rg -JA40/30/7.5i -O -K -W2p,purple >> $ps
# Try plot 10 degree small circle around P
gmt project -C$C -T$P -G1/10 -L-20/60 | gmt psxy -Rg -JA40/30/7.5i -O -K -W2p,sandybrown >> $ps
#
# Test small circles forced to go through center C and the second point (here we use E = P)
# This involves computing the pole for this circle; its coordinates are reported in the
# segment ps=small.ps
# Try plot 80 degree small half circle only through C and E
gmt project -C$C -E$P -G1/80+ -L-90/90 > tmp
gmt psxy -R -J -O -K -W2p,blue tmp >> $ps
grep Pole tmp | $AWK '{print $5, $6}' | gmt psxy -Rg -JA40/30/7.5i -O -K -Sc0.1i -Gblue >> $ps
# Try plot 60 degree small half circle through C and E
gmt project -C$C -E$P -G1/60+ -L-90/90 > tmp
gmt psxy -R -J -O -K -W2p,magenta tmp >> $ps
grep Pole tmp | $AWK '{print $5, $6}' | gmt psxy -Rg -JA40/30/7.5i -O -K -Sc0.1i -Gmagenta >> $ps
# Try plot 40 degree small full circle through C and E
gmt project -C$C -E$P -G1/40+ -L-180/180 > tmp
gmt psxy -R -J -O -K -W2p,tan tmp >> $ps
grep Pole tmp | $AWK '{print $5, $6}' | gmt psxy -Rg -JA40/30/7.5i -O -K -Sc0.1i -Gtan >> $ps
# Try plot 33 degree small half circle through C and E
gmt project -C$C -E$P -G1/33+ -L-90/90 > tmp
gmt psxy -R -J -O -K -W2p,brown tmp >> $ps
grep Pole tmp | $AWK '{print $5, $6}' | gmt psxy -Rg -JA40/30/7.5i -O -K -Sc0.1i -Gbrown >> $ps
# Plot P and C and create circle through P and C
gmt project -C$C -E$P -G1 -L-180/180 | gmt psxy -Rg -JA40/30/7.5i -O -K -W2p,- >> $ps
echo 85 40 | gmt psxy -Rg -JA40/30/7.5i -O -K -Sa0.3i -Gblack >> $ps
echo 15 15 | gmt psxy -Rg -JA40/30/7.5i -O -K -Sa0.3i -Gblack >> $ps
echo 85 40 P | gmt pstext -Rg -JA40/30/7.5i -O -K -D0/-0.2i -F+f14p >> $ps
echo 15 15 C | gmt pstext -Rg -JA40/30/7.5i -O -K -D0/-0.2i -F+f14p >> $ps

# The end
gmt psbasemap -R -J -O -B30g30 >> $ps
