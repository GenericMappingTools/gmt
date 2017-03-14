#!/bin/bash
#		GMT EXAMPLE 03
#		$Id$
#
# Purpose:	Resample track data, do spectral analysis, and plot
# GMT modules:	filter1d, fitcircle, gmtconvert, gmtinfo, project, sample1d
# GMT modules:	spectrum1d, trend1d, pshistogram, psxy, pstext
# Unix progs:	echo, rm
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
gmt set GMT_FFT kiss
cpos=`gmt fitcircle sat.xyg -L2 -Fm --IO_COL_SEPARATOR=/`
ppos=`gmt fitcircle sat.xyg -L2 -Fn --IO_COL_SEPARATOR=/`
#
# Now we use "gmt project" to project the data in both sat.xyg and ship.xyg
# into data.pg, where g is the same and p is the oblique longitude around
# the great circle.  We use -Q to get the p distance in kilometers, and -S
# to sort the output into increasing p values.
#
gmt project  sat.xyg -C$cpos -T$ppos -S -Fpz -Q > sat.pg
gmt project ship.xyg -C$cpos -T$ppos -S -Fpz -Q > ship.pg
#
# The gmtinfo utility will report the minimum and maximum values for all columns. 
# We use this information first with a large -I value to find the appropriate -R
# to use to plot the .pg data. 
#
R=`gmt info -I100/25 sat.pg ship.pg`
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
# typically spaced using gmtmath DIFF to get the delta-p between points and view it with 
# "gmt pshistogram".
#
gmt math ship.pg -T -i0 DIFF = | gmt pshistogram  -W0.1 -Gblack \
	-JX3i -K -X2i -Y1.5i -B0 -B+t"Ship" -UL/-1.75i/-1.25i/"Example 3b in Cookbook" \
	> example_03b.ps
gmt math sat.pg -T -i0 DIFF = | gmt pshistogram  -W0.1 -Gblack \
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
# both of them.  This is a job for gmt info -L -Af.
#
bounds=`gmt info ship.pg sat.pg -I1 -Af -L -C -i0  --IO_COL_SEPARATOR=/`
#
# Now we can use $bounds in gmt math to make a sampling points file for gmt sample1d:
gmt math -T$bounds/1 -N1/0 T = samp.x
#
# Now we can resample the gmt projected satellite data:
#
gmt sample1d sat.pg -Nsamp.x > samp_sat.pg
#
# For reasons above, we use gmt filter1d to pre-treat the ship data.  We also need to sample
# it because of the gaps > 1 km we found.  So we use gmt filter1d | gmt sample1d.  We also
# use the -E on gmt filter1d to use the data all the way out to bounds :
#
gmt filter1d ship.pg -Fm1 -T$bounds/1 -E | gmt sample1d -Nsamp.x > samp_ship.pg
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
gmt convert -A samp_ship.pg samp_sat.pg -o1,3 | gmt spectrum1d -S256 -D1 -W -C -T
# 
# Now we want to plot the spectra. The following commands will plot the ship and sat 
# power in one diagram and the coherency on another diagram, both on the same page.  
# We end by adding a map legends and some labels on the plots.
# For that purpose we often use -Jx1i and specify positions in inches directly:
#
gmt psxy spectrum.coh -Bxa1f3p+l"Wavelength (km)" -Bya0.25f0.05+l"Coherency@+2@+" \
	-BWeSn+g240/255/240 -JX-4il/3.75i -R1/1000/0/1 -P -K -X2.5i -Sc0.07i -Gpurple \
	-Ey+p0.5p -Y1.5i > $ps
echo "Coherency@+2@+" | gmt pstext -R -J -F+cTR+f18p,Helvetica-Bold -Dj0.1i \
	-O -K >> $ps
gmt psxy spectrum.xpower -Bxa1f3p -Bya1f3p+l"Power (mGal@+2@+km)" \
	-BWeSn+t"Ship and Satellite Gravity"+g240/255/240 \
	-Gred -ST0.07i -O -R1/1000/0.1/10000 -JX-4il/3.75il -Y4.2i -K -Ey+p0.5p >> $ps
gmt psxy spectrum.ypower -R -JX -O -K -Gblue -Sc0.07i -Ey+p0.5p >> $ps
echo "Input Power" | gmt pstext -R0/4/0/3.75 -Jx1i -F+cTR+f18p,Helvetica-Bold -Dj0.1i -O -K >> $ps
gmt pslegend -R -J -O -DjBL+w1.2i+o0.25i -F+gwhite+pthicker --FONT_ANNOT_PRIMARY=14p,Helvetica-Bold << EOF >> $ps
S 0.1i T 0.07i red - 0.3i Ship
S 0.1i c 0.07i blue - 0.3i Satellite
EOF
#
# Now we wonder if removing that large feature at 250 km would make any difference.
# We could throw away a section of data with awk or sed or head and tail, but we
# demonstrate the use of "gmt trend1d" to identify outliers instead.  We will fit a
# straight line to the samp_ship.pg data by an iteratively-reweighted method and
# save the weights on output.  Then we will plot the weights and see how things
# look:
#
gmt trend1d -Fxw -Np1+r samp_ship.pg > samp_ship.xw
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
gmt trend1d -Fxrw -Np1+r samp_ship.pg | gmt select -Z0/0.6 -o0,1 -Iz \
	| gmt sample1d -Nsamp.x > samp2_ship.pg
gmt trend1d -Fxrw -Np1+r samp_sat.pg  | gmt select -Z0/0.6 -o0,1 -Iz \
	| gmt sample1d -Nsamp.x > samp2_sat.pg
#
# We plot these to see how they look:
#
R=`gmt info -I100/25 samp2_sat.pg samp2_ship.pg`
gmt psxy $R -JX8i/5i -X2i -Y1.5i -K -Wthick \
	-Bxa500f100+l"Distance along great circle" -Bya50f25+l"Gravity anomaly (mGal)" \
	-BWeSn -UL/-1.75i/-1.25i/"Example 3e in Cookbook" samp2_sat.pg > example_03e.ps
gmt psxy -R -JX -O -Sp0.03i samp2_ship.pg >> example_03e.ps
#
# Now we do the cross-spectral analysis again.  Comparing this plot (example_03e.ps) with
# the previous one (example_03d.ps) we see that throwing out the large feature has reduced
# the power in both data sets and reduced the coherency at wavelengths between 20--60 km.
#
gmt convert -A samp2_ship.pg samp2_sat.pg -o1,3 | gmt spectrum1d -S256 -D1 -W -C -T
# 
gmt psxy spectrum.coh -Bxa1f3p+l"Wavelength (km)" -Bya0.25f0.05+l"Coherency@+2@+" -BWeSn \
	-JX-4il/3.75i -R1/1000/0/1 -UL/-2.25i/-1.25i/"Example 3f in Cookbook" -P -K -X2.5i \
	-Sc0.07i -Gblack -Ey+p0.5p -Y1.5i > example_03f.ps
echo "Coherency@+2@+" | gmt pstext -R -J -F+cTR+f18p,Helvetica-Bold -Dj0.1i \
	-O -K -Wthicker -C0.1i >> example_03f.ps
gmt psxy -Bxa1f3p -Bya1f3p+l"Power (mGal@+2@+km)" -BWeSn+t"Ship and Satellite Gravity" \
	spectrum.xpower -ST0.07i -O -R1/1000/0.1/10000 -JX-4il/3.75il -Y4.2i -K -Ey+p0.5p \
	>> example_03f.ps
gmt psxy spectrum.ypower -R -J -O -K -Gblack -Sc0.07i -Ey+p0.5p >> example_03f.ps
echo "Input Power" | gmt pstext -R -J -F+cTR+f18p,Helvetica-Bold -Dj0.1i \
	-O -K -Wthicker -C0.1i >> example_03f.ps
gmt pslegend -R0/4/0/3.75 -Jx -O -DjBL+w1.2i+o0.25i -F+glightgray+pthicker \
	--FONT_ANNOT_PRIMARY=14p,Helvetica-Bold << EOF >> example_03f.ps
S 0.1i T 0.07i black - 0.3i Ship
S 0.1i c 0.07i black - 0.3i Satellite
EOF
#
rm -f report tmp samp* *.pg *.extr spectrum.*
