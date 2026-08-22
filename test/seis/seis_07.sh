#!/usr/bin/env bash
#
# Check gmt psmeca for plotting beach balls
# Verified by K. Feigl on 2015/8/6.  Comments left below.

ps=seis_07.ps

size=1.0c

#THOUGHTS:
#echo 2.0 1.0 0.0  90 90  90 270  0  90 4 23 0 0 Dip slip    | gmt psmeca -Sc${size} -L1 -Gblack -R -J -K -O >> $ps
#echo 2.0 1.0 0.0  90 90  90  90  0 -90 4 23 0 0 Dip slip    | gmt psmeca -Sc${size} -L1 -Gblack -R -J -K -O >> $ps
# Question: The previous two lines don't give the same result.. Maybe there could be a warning to not use the
#same strike twice?
# Answer via K. Feigl 2015/8/6: The -Sc option describes two planes with 6 numbers where 3 are enough.
# So, no warning if two planes are not compatible, i.e., orthogonal.

#TESTING AKI OPTION -Sa  - PURE STRIKE SLIP
# Explosion
# - not possible
# Implosion
# - not possible
# Right lateral strike slip
echo 1.0 1.0 0.0  0 90  0 5 0 0 Strike slip | gmt psmeca -Sa${size} -L1 -Gblack -R-1/6/-1/2 -JM14c -P -Y20c -B2 -K > $ps
# Rotated strike slip
echo 1.0 0.0 0.0  45 90 180 5 0 0 Strike slip 45 deg | gmt psmeca -Sa${size} -L1 -Gblack -R -J -K -O >> $ps
# Dip slip
echo 2.0 1.0 0.0  90  0 -90 5 0 0 Dip slip    | gmt psmeca -Sa${size} -L1 -Gblack -R -J -K -O >> $ps
# Dip slip
#echo 2.0 0.0 0.0   0 90  90 5 0 0 Dip slip    | gmt psmeca -Sa${size} -L1 -Gblack -R -J -K -O >> $ps
echo 2.0 0.0 0.0   0  0 -90 5 0 0 Dip slip    | gmt psmeca -Sa${size} -L1 -Gblack -R -J -K -O >> $ps
# Thrust
echo 3.0 1.0 0.0  90 45 90 5 0 0 Thrust      | gmt psmeca -Sa${size} -L1 -Gblack -R -J -K -O >> $ps
# Thrust
echo 3.0 0.0 0.0   0 45 90 5 0 0 Thrust      | gmt psmeca -Sa${size} -L1 -Gblack -R -J -K -O >> $ps
# Horizontal CLVD
# - not possible
# Horizontal CLVD
# - not possible
# Vertical CLVD
# - not possible
# Vertical CLVD
# - not possible

#TESTING HARVARD OPTION -Sc - two fault planes

# Right lateral Strike Slip

# Rotated strike Slip
# Explosion
# - not possible
# Implosion
# - not possible
# Right lateral strike slip
echo 1.0 1.0 0.0  90 90 180   0 90   0 4 23 0 0 Right Strike Slip | gmt psmeca -Sc${size} -L1 -Gblack -R -JM14c -Y-8c -P -B2 -K -O >> $ps
# Rotated strike slip
echo 1.0 0.0 0.0  45 90 180 135 90   0 4 23 0 0 Strike slip 45 deg | gmt psmeca -Sc${size} -L1 -Gblack -R -J -K -O >> $ps
# Dip slip
echo 2.0 1.0 0.0  90 90  90 270  0  90 4 23 0 0 Dip slip    | gmt psmeca -Sc${size} -L1 -Gblack -R -J -K -O >> $ps
# Dip slip
echo 2.0 0.0 0.0   0 90  90 180  0  90 4 23 0 0 Dip slip    | gmt psmeca -Sc${size} -L1 -Gblack -R -J -K -O >> $ps
# Thrust
echo 3.0 1.0 0.0  90 45  90 270 45  90 4 23 0 0 Thrust      | gmt psmeca -Sc${size} -L1 -Gblack -R -J -K -O >> $ps
# Thrust
echo 3.0 0.0 0.0   0 45  90 180 45  90 4 23 0 0 Thrust      | gmt psmeca -Sc${size} -L1 -Gblack -R -J -K -O >> $ps
# Horizontal CLVD
# - not possible
# Horizontal CLVD
# - not possible
# Vertical CLVD
# - not possible
# Vertical CLVD
# - not possible

#TESTING HARVARD OPTION -Sm - moment tensor elements
#Following Dahlen and Tromp (1998) page 174
# Explosion
echo 0.0 1.0 0.0  1.0  1.0  1.0  0.0  0.0  0.0 23 0 0 Explosion   | gmt psmeca -Sm${size} -L1 -Gblack -R -J -Y-8c -P -B2 -K -O >> $ps
# Implosion
echo 0.0 0.0 0.0 -1.0 -1.0 -1.0  0.0  0.0  0.0 23 0 0 Implosion   | gmt psmeca -Sm${size} -L1 -Gblack -R -J -K -O >> $ps
# Right lateral strike slip
echo 1.0 1.0 0.0  0.0  0.0  0.0  0.0  0.0 -1.0 23 0 0 Strike slip | gmt psmeca -Sm${size} -L1 -Gblack -R -J -K -O >> $ps
# Rotated strike slip
echo 1.0 0.0 0.0  0.0  1.0 -1.0  0.0  0.0  0.0 23 0 0 Strike slip 45 deg | gmt psmeca -Sm${size} -L1 -Gblack -R -J -K -O >> $ps
# Dip slip
echo 2.0 1.0 0.0  0.0  0.0  0.0  1.0  0.0  0.0 23 0 0 Dip slip    | gmt psmeca -Sm${size} -L1 -Gblack -R -J -K -O >> $ps
# Dip slip
echo 2.0 0.0 0.0  0.0  0.0  0.0  0.0  1.0  0.0 23 0 0 Dip slip    | gmt psmeca -Sm${size} -L1 -Gblack -R -J -K -O >> $ps
# Thrust
echo 3.0 1.0 0.0  1.0 -1.0  0.0  0.0  0.0  0.0 23 0 0 Thrust      | gmt psmeca -Sm${size} -L1 -Gblack -R -J -K -O >> $ps
# Thrust
echo 3.0 0.0 0.0  1.0  0.0 -1.0  0.0  0.0  0.0 23 0 0 Thrust      | gmt psmeca -Sm${size} -L1 -Gblack -R -J -K -O >> $ps
# Horizontal CLVD
echo 4.0 1.0 0.0  1.0  1.0 -2.0  0.0  0.0  0.0 23 0 0 Horiz. CLVD | gmt psmeca -Sm${size} -L1 -Gblack -R -J -K -O >> $ps
# Horizontal CLVD
echo 4.0 0.0 0.0  1.0 -2.0  1.0  0.0  0.0  0.0 23 0 0 Horiz. CLVD | gmt psmeca -Sm${size} -L1 -Gblack -R -J -K -O >> $ps
# Vertical CLVD-L1
echo 5.0 1.0 0.0 -2.0  1.0  1.0  0.0  0.0  0.0 23 0 0 Vertical CLVD | gmt psmeca -Sm${size} -L1 -Gblack -R -K -J -O >> $ps
# Vertical CLVD
echo 5.0 0.0 0.0  2.0 -1.0 -1.0  0.0  0.0  0.0 23 0 0 Vertical CLVD | gmt psmeca -Sm${size} -L1 -Gblack -R -J -K -O >> $ps
gmt pstext -R -J -O -F+f12p,+cLB+t"Dahlen and Tromp (1998) page 174" -Dj0.1i  >> $ps
