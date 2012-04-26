#!/bin/bash
#		GMT EXAMPLE 21
#		$Id$
#
# Purpose:	Plot a time-series
# GMT progs:	gmtset, gmtconvert, minmax, psbasemap, psxy 
# Unix progs:	cut, echo
#
ps=example_21.ps

# File has time stored as dd-Mon-yy so set input format to match it

gmtset FORMAT_DATE_IN dd-o-yy FORMAT_DATE_MAP o FONT_ANNOT_PRIMARY +10p
gmtset FORMAT_TIME_PRIMARY_MAP abbreviated PS_CHAR_ENCODING ISOLatin1+

# Pull out a suitable region string in yyy-mm-dd format

minmax -fT -I50 -C RHAT_price.csv > RHAT.info
w=`cut -f1 RHAT.info`
e=`cut -f2 RHAT.info`
s=`cut -f3 RHAT.info`
n=`cut -f4 RHAT.info`
R="-R$w/$e/$s/$n"

# Lay down the basemap:

psbasemap $R -JX9i/6i -K -U"Example 21 in Cookbook" -Bs1Y/WSen \
   -Bpa3Of1o/50:=\$::."RedHat (RHT) Stock Price Trend since IPO":WSen+glightgreen > $ps

# Plot main window with open price as red line over yellow envelope of low/highs

gmtset FORMAT_DATE_OUT dd-o-yy
gmtconvert -o0,2 -f0T RHAT_price.csv > RHAT.env
gmtconvert -o0,3 -f0T -I -T RHAT_price.csv >> RHAT.env
psxy -R -J -Gyellow -O -K RHAT.env >> $ps
psxy -R -J RHAT_price.csv -Wthin,red -O -K >> $ps

# Draw P Wessel's purchase price as line and label it.  Note we temporary switch
# back to default yyyy-mm-dd format since that is what minmax gave us.

echo "05-May-00	0" > RHAT.pw
echo "05-May-00	300" >> RHAT.pw
psxy -R -J RHAT.pw -Wthinner,- -O -K >> $ps
echo "01-Jan-99	25" > RHAT.pw
echo "01-Jan-02	25" >> RHAT.pw
psxy -R -J RHAT.pw -Wthick,- -O -K >> $ps
gmtset FORMAT_DATE_IN yyyy-mm-dd
echo "$w 25 PW buy" | pstext -R -J -O -K -D1.5i/0.05i -N -F+f12p,Bookman-Demi+jLB >> $ps
gmtset FORMAT_DATE_IN dd-o-yy

# Draw P Wessel's sales price as line and label it.

echo "25-Jun-07	0" > RHAT.pw
echo "25-Jun-07	300" >> RHAT.pw
psxy -R -J RHAT.pw -Wthinner,- -O -K >> $ps
echo "01-Aug-06	23.8852" > RHAT.pw
echo "01-Jan-08	23.8852" >> RHAT.pw
psxy -R -J RHAT.pw -Wthick,- -O -K >> $ps
gmtset FORMAT_DATE_IN yyyy-mm-dd
echo "$e 23.8852 PW sell" | pstext -R -J -O -K -Dj0.8i/0.05i -N -F+f12p,Bookman-Demi+jRB >> $ps
gmtset FORMAT_DATE_IN dd-o-yy

# Get smaller region for insert for trend since 2004

R="-R2004T/$e/$s/40"

# Lay down the basemap, using Finnish annotations and place the insert in the upper right

psbasemap --TIME_LANGUAGE=fi $R -JX6i/3i -Bpa3Of3o/10:=\$:ESw+glightblue -Bs1Y/ \
	-O -K -X3i -Y3i >> $ps

# Again, plot close price as red line over yellow envelope of low/highs

psxy -R -J -Gyellow -O -K RHAT.env >> $ps
psxy -R -J RHAT_price.csv -Wthin,red -O -K >> $ps

# Draw P Wessel's sales price as dashed line

psxy -R -J RHAT.pw -Wthick,- -O -K >> $ps

# Mark sales date

echo "25-Jun-07	0" > RHAT.pw
echo "25-Jun-07	300" >> RHAT.pw
psxy -R -J RHAT.pw -Wthinner,- -O >> $ps

# Clean up after ourselves:

rm -f RHAT.*
