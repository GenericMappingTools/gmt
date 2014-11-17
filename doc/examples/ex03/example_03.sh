#!/bin/bash
#		GMT EXAMPLE 03
#		$Id$
#
# Purpose:	Resample track data, do spectral analysis, and plot
# GMT progs:	filter1d, fitcircle, gmtinfo, project, sample1d
# GMT progs:	spectrum1d, trend1d, pshistogram, psxy, pstext
# Unix progs:	$AWK, cat, echo, head, paste, rm, tail
#
# This example begins with data files "ship.xyg" and "sat.xyg" which
# are measurements of a quantity "g" (a "gravity anomaly" which is an
# anomalous increase or decrease in the magnitude of the acceleration
# of gravity at sea level).  g is measured at a sequence of points "x,y"
# which in this case are "longitude,latitude".  The "sat.xyg" data were
# obtained by a satellite and the sequence of points lies almost along
# a great circle.  The "ship.xyg" data were obtained by a ship which
# tried to follow the satellite's path but deviated from it in places.
# Thus the two data sets are not measured at the same of points,
# and we use various GMT tools to facilitate their comparison.
# The main illustration (example_03.ps) are accompanied with 5 support
# plots (03a-f) showing data distributions and various intermediate steps.
#
# First, we use "gmt fitcircle" to find the parameters of a great circle
# most closely fitting the x,y points in "sat.xyg":
#
ps=example_03.ps
gmt fitcircle sat.xyg -L2 > report
cposx=`grep "L2 Average Position" report | cut -f1` 
cposy=`grep "L2 Average Position" report | cut -f2` 
pposx=`grep "L2 N Hemisphere" report | cut -f1` 
pposy=`grep "L2 N Hemisphere" report | cut -f2` 
#
# Now we use "gmt project" to gmt project the data in both sat.xyg and ship.xyg
# into data.pg, where g is the same and p is the oblique longitude around
# the great circle.  We use -Q to get the p distance in kilometers, and -S
# to sort the output into increasing p values.
#
gmt project  sat.xyg -C$cposx/$cposy -T$pposx/$pposy -S -Fpz -Q > sat.pg
gmt project ship.xyg -C$cposx/$cposy -T$pposx/$pposy -S -Fpz -Q > ship.pg
#
# The gmtinfo utility will report the minimum and maximum values for all columns. 
# We use this information first with a large -I value to find the appropriate -R
# to use to plot the .pg data. 
#
R=`cat sat.pg ship.pg | gmt info -I100/25`
gmt psxy $R -UL/-1.75i/-1.25i/"Example 3a in Cookbook" -BWeSn \
	-Bxa500f100+l"Distance along great circle" -Bya100f25+l"Gravity anomaly (mGal)" \
	-JX8i/5i -X2i -Y1.5i -K -Wthick sat.pg > example_03a.ps
gmt psxy -R -JX -O -Sp0.03i ship.pg >> example_03a.ps
#
# From this plot we see that the ship data have some "spikes" and also greatly
# differ from the satellite data at a point about p ~= +250 km, where both of
# them show a very large anomaly.
#
# To facilitate comparison of the two with a cross-spectral analysis using "gmt spectrum1d",
# we resample both data sets at intervals of 1 km.  First we find out how the data are
# typically spaced using $AWK to get the delta-p between points and view it with 
# "gmt pshistogram".
#
$AWK '{ if (NR > 1) print $1 - last1; last1=$1; }' ship.pg | gmt pshistogram  -W0.1 -Gblack \
	-JX3i -K -X2i -Y1.5i -B0 -B+t"Ship" -UL/-1.75i/-1.25i/"Example 3b in Cookbook" \
	> example_03b.ps
$AWK '{ if (NR > 1) print $1 - last1; last1=$1; }' sat.pg  | gmt pshistogram  -W0.1 -Gblack \
	-JX3i -O -X5i -B0 -B+t"Sat" >> example_03b.ps
#
# This experience shows that the satellite values are spaced fairly evenly, with
# delta-p between 3.222 and 3.418.  The ship values are spaced quite unevenly, with
# delta-p between 0.095 and 9.017.  This means that when we want 1 km even sampling,
# we can use "gmt sample1d" to interpolate the sat data, but the same procedure applied
# to the ship data could alias information at shorter wavelengths.  So we have to use
# "gmt filter1d" to resample the ship data.  Also, since we observed spikes in the ship
# data, we use a median filter to clean up the ship values.  We will want to use "paste"
# to put the two sampled data sets together, so they must start and end at the same
# point, without NaNs.  So we want to get a starting and ending point which works for
# both of them.  This is a job for gmt gmtmath UPPER/LOWER.
#
head -1 ship.pg > tmp
head -1 sat.pg >> tmp
sampr1=`gmt gmtmath tmp -Ca -Sf -o0 UPPER CEIL =`
tail -1 ship.pg > tmp
tail -1 sat.pg >> tmp 
sampr2=`gmt gmtmath tmp -Ca -Sf -o0 LOWER FLOOR =`
#
# Now we can use sampr1|2 in gmt gmtmath to make a sampling points file for gmt sample1d:
gmt gmtmath -T$sampr1/$sampr2/1 -N1/0 T = samp.x
#
# Now we can resample the gmt projected satellite data:
#
gmt sample1d sat.pg -Nsamp.x > samp_sat.pg
#
# For reasons above, we use gmt filter1d to pre-treat the ship data.  We also need to sample
# it because of the gaps > 1 km we found.  So we use gmt filter1d | gmt sample1d.  We also
# use the -E on gmt filter1d to use the data all the way out to sampr1/sampr2 :
#
gmt filter1d ship.pg -Fm1 -T$sampr1/$sampr2/1 -E | gmt sample1d -Nsamp.x > samp_ship.pg
#
# Now we plot them again to see if we have done the right thing:
#
gmt psxy $R -JX8i/5i -X2i -Y1.5i -K -Wthick samp_sat.pg \
	-Bxa500f100+l"Distance along great circle" -Bya100f25+l"Gravity anomaly (mGal)" \
	-BWeSn -UL/-1.75i/-1.25i/"Example 3c in Cookbook" > example_03c.ps
gmt psxy -R -JX -O -Sp0.03i samp_ship.pg >> example_03c.ps
#
# Now to do the cross-spectra, assuming that the ship is the input and the sat is the output 
# data, we do this:
# 
gmt gmtconvert -A samp_ship.pg samp_sat.pg -o1,3 | gmt spectrum1d -S256 -D1 -W -C > /dev/null
# 
# Now we want to plot the spectra. The following commands will plot the ship and sat 
# power in one diagram and the coherency on another diagram, both on the same page.  
# Note the extended use of gmt pstext and gmt psxy to put labels and legends directly on the
# plots. For that purpose we often use -Jx1i and specify positions in inches directly:
#
gmt psxy spectrum.coh -Bxa1f3p+l"Wavelength (km)" -Bya0.25f0.05+l"Coherency@+2@+" \
	-BWeSn+g240/255/240 -JX-4il/3.75i -R1/1000/0/1 -P -K -X2.5i -Sc0.07i -Gpurple \
	-Ey/0.5p -Y1.5i > $ps
echo "3.85 3.6 Coherency@+2@+" | gmt pstext -R0/4/0/3.75 -Jx1i -F+f18p,Helvetica-Bold+jTR \
	-O -K >> $ps
gmt psxy spectrum.xpower -Bxa1f3p -Bya1f3p+l"Power (mGal@+2@+km)" \
	-BWeSn+t"Ship and Satellite Gravity"+g240/255/240 \
	-Gred -ST0.07i -O -R1/1000/0.1/10000 -JX-4il/3.75il -Y4.2i -K -Ey/0.5p >> $ps
gmt psxy spectrum.ypower -R -JX -O -K -Gblue -Sc0.07i -Ey/0.5p >> $ps
echo "3.9 3.6 Input Power" | gmt pstext -R0/4/0/3.75 -Jx -F+f18p,Helvetica-Bold+jTR -O -K >> $ps
gmt psxy -R -Jx -O -K -Gwhite -L -Wthicker >> $ps << END
0.25	0.25
1.4	0.25
1.4	0.9
0.25	0.9
END
echo "0.4 0.7" | gmt psxy -R -Jx -O -K -ST0.07i -Gred >> $ps
echo "0.5 0.7 Ship" | gmt pstext -R -Jx -F+f14p,Helvetica-Bold+jLM -O -K >> $ps
echo "0.4 0.4" | gmt psxy -R -Jx -O -K -Sc0.07i -Gblue >> $ps
echo "0.5 0.4 Satellite" | gmt pstext -R -Jx -F+f14p,Helvetica-Bold+jLM -O >> $ps
#
# Now we wonder if removing that large feature at 250 km would make any difference.
# We could throw away a section of data with $AWK or sed or head and tail, but we
# demonstrate the use of "gmt trend1d" to identify outliers instead.  We will fit a
# straight line to the samp_ship.pg data by an iteratively-reweighted method and
# save the weights on output.  Then we will plot the weights and see how things
# look:
#
gmt trend1d -Fxw -N2r samp_ship.pg > samp_ship.xw
gmt psxy $R -JX8i/4i -X2i -Y1.5i -K -Sp0.03i \
	-Bxa500f100+l"Distance along great circle" -Bya100f25+l"Gravity anomaly (mGal)" \
	-BWeSn -UL/-1.75i/-1.25i/"Example 3d in Cookbook" samp_ship.pg > example_03d.ps
R=`gmt info samp_ship.xw -I100/1.1`
gmt psxy $R -JX8i/1.1i -O -Y4.25i -Bxf100 -Bya0.5f0.1+l"Weight" -BWesn -Sp0.03i \
	samp_ship.xw >> example_03d.ps
#
# From this we see that we might want to throw away values where w < 0.6.  So we try that,
# and this time we also use gmt trend1d to return the residual from the model fit (the 
# de-trended data):
gmt trend1d -Fxrw -N2r samp_ship.pg | $AWK '{ if ($3 > 0.6) print $1, $2 }' \
	| gmt sample1d -Nsamp.x > samp2_ship.pg
gmt trend1d -Fxrw -N2r samp_sat.pg  | $AWK '{ if ($3 > 0.6) print $1, $2 }' \
	| gmt sample1d -Nsamp.x > samp2_sat.pg
#
# We plot these to see how they look:
#
R=`cat samp2_sat.pg samp2_ship.pg | gmt info -I100/25`
gmt psxy $R -JX8i/5i -X2i -Y1.5i -K -Wthick \
	-Bxa500f100+l"Distance along great circle" -Bya50f25+l"Gravity anomaly (mGal)" \
	-BWeSn -UL/-1.75i/-1.25i/"Example 3e in Cookbook" samp2_sat.pg > example_03e.ps
gmt psxy -R -JX -O -Sp0.03i samp2_ship.pg >> example_03e.ps
#
# Now we do the cross-spectral analysis again.  Comparing this plot (example_03e.ps) with
# the previous one (example_03d.ps) we see that throwing out the large feature has reduced
# the power in both data sets and reduced the coherency at wavelengths between 20--60 km.
#
gmt gmtconvert -A samp2_ship.pg samp2_sat.pg -o1,3 | gmt spectrum1d -S256 -D1 -W -C > /dev/null
# 
gmt psxy spectrum.coh -Bxa1f3p+l"Wavelength (km)" -Bya0.25f0.05+l"Coherency@+2@+" -BWeSn \
	-JX-4il/3.75i -R1/1000/0/1 -UL/-2.25i/-1.25i/"Example 3f in Cookbook" -P -K -X2.5i \
	-Sc0.07i -Gblack -Ey/0.5p -Y1.5i > example_03f.ps
echo "3.85 3.6 Coherency@+2@+" | gmt pstext -R0/4/0/3.75 -Jx -F+f18p,Helvetica-Bold+jTR -O \
	-K >> example_03f.ps
cat > box.d << END
2.375	3.75
2.375	3.25
4	3.25
END
gmt psxy -R -Jx -O -K -Wthicker box.d >> example_03f.ps
gmt psxy -Bxa1f3p -Bya1f3p+l"Power (mGal@+2@+km)" -BWeSn+t"Ship and Satellite Gravity" \
	spectrum.xpower -ST0.07i -O -R1/1000/0.1/10000 -JX-4il/3.75il -Y4.2i -K -Ey/0.5p \
	>> example_03f.ps
gmt psxy spectrum.ypower -R -JX -O -K -Gblack -Sc0.07i -Ey/0.5p >> example_03f.ps
echo "3.9 3.6 Input Power" | gmt pstext -R0/4/0/3.75 -Jx -F+f18p,Helvetica-Bold+jTR -O \
	-K >> example_03f.ps
gmt psxy -R -Jx -O -K -Wthicker box.d >> example_03f.ps
gmt psxy -R -Jx -O -K -Glightgray -L -Wthicker >> example_03f.ps << END
0.25	0.25
1.4	0.25
1.4	0.9
0.25	0.9
END
echo "0.4 0.7" | gmt psxy -R -Jx -O -K -ST0.07i -Gblack >> example_03f.ps
echo "0.5 0.7 Ship" | gmt pstext -R -Jx -F+f14p,Helvetica-Bold+jLM -O -K >> example_03f.ps
echo "0.4 0.4" | gmt psxy -R -Jx -O -K -Sc0.07i -Gblack >> example_03f.ps
echo "0.5 0.4 Satellite" | gmt pstext -R -Jx -F+f14p,Helvetica-Bold+jLM -O >> example_03f.ps
#
rm -f box.d report tmp samp* *.pg *.extr spectrum.*
