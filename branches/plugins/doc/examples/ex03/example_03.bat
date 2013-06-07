REM
REM		GMT EXAMPLE 03
REM
REM		$Id$
REM
echo GMT EXAMPLE 03
set ps=example_03.ps
REM This version of example 3 is short and ugly.  We simply do not
REM have access to all the UNIX tools needed. See job03.sh for details.
REM First, we use "gmt fitcircle" to find the parameters of a great circle
REM most closely fitting the x,y points in "sat.xyg":
REM We find that center is 330.169/-18.4207 and pole is 52.7452/21.204
REM
REM Now we use "gmt project" to project the data in both sat.xyg and ship.xyg
REM into data.pg, where g is the same and p is the oblique longitude around
REM the great circle.  We use -Q to get the p distance in kilometers, and -S
REM to sort the output into increasing p values.
REM
gmt project  sat.xyg -C330.169/-18.4207 -T52.7452/21.204 -S -Fpz -Q > sat.pg
gmt project ship.xyg -C330.169/-18.4207 -T52.7452/21.204 -S -Fpz -Q > ship.pg
REM Now we can use sampr in gawk to make a sampling points file for gmt sample1d:
gmt gmtmath -T-1167/1169/1 -N1/0 = samp.x
REM
REM Now we can resample the gmt projected satellite data:
REM
gmt sample1d sat.pg -Nsamp.x > samp_sat.pg
REM
REM For reasons above, we use gmt filter1d to pre-treat the ship data.  We also need to sample it
REM because of the gaps over 1 km we found.  So we use gmt filter1d and gmt sample1d.  We also use the -E
REM on gmt filter1d to use the data all the way out to sampr1/sampr2 :
REM
gmt filter1d ship.pg -Fm1 -T-1167/1169/1 -E | gmt sample1d -Nsamp.x > samp_ship.pg
REM Now to do the cross-spectra, assuming that the ship is the input and the sat is the output 
REM data, we do this:
REM 
gmt spectrum1d -S256 -D1 -W -C shipsat.dos
REM 
REM Now we want to plot the spectra.  The following commands will plot the ship and sat 
REM power in one diagram and the coherency on another diagram,  both on the same page.  
REM Note the extended use of gmt pstext and gmt psxy to put labels and legends directly on the plots.  
REM For that purpose we often use -Jx1i and specify positions in inches directly:
REM
gmt psxy spectrum.coh -Bxa1f3p+l"Wavelength (km)" -Bya0.25f0.05+l"Coherency@+2@+" -BWeSn+g240/255/240 -JX-4il/3.75i -R1/1000/0/1 -P -K -X2.5i -Sc0.07i -G0 -Ey/0.5p -Y1.5i > %ps%
echo 3.85 3.6 Coherency@+2@+ | gmt pstext -R0/4/0/3.75 -Jx1i -F+f18p,Helvetica-Bold+jTR -O -K >> %ps%
echo 2.375 3.75 > box.d
echo 2.375 3.25 >> box.d
echo 4 3.25 >> box.d
gmt psxy -R -Jx -O -K -Wthicker box.d >> %ps%
gmt psxy -Gblack -ST0.07i -O -Bxa1f3p -Bya1f3p+l"Power (mGal@+2@+km)" -BWeSn+t"Ship and Satellite Gravity"+g240/255/240 spectrum.xpower -R1/1000/0.1/10000 -JX-4il/3.75il -Y4.2i -K -Ey/0.5p >> %ps%
gmt psxy spectrum.ypower -R -JX -O -K -G0 -Sc0.07i -Ey/0.5p >> %ps%
echo 3.9 3.6 Input Power | gmt pstext -R0/4/0/3.75 -Jx -F+f18p,Helvetica-Bold+jTR -O -K >> %ps%
gmt psxy -R -Jx -O -K -Wthicker box.d >> %ps%
echo 0.25 0.25 > box.d
echo 1.4 0.25 >> box.d
echo 1.4 0.9 >> box.d
echo 0.25 0.9 >> box.d
gmt psxy -R -Jx -O -K -G240 -L -Wthicker box.d >> %ps%
echo 0.4 0.7 | gmt psxy -R -Jx -O -K -ST0.07i -G0 >> %ps%
echo 0.5 0.7 Ship | gmt pstext -R -Jx -F+f14p,Helvetica-Bold+jLM -O -K >> %ps%
echo 0.4 0.4 | gmt psxy -R -Jx -O -K -Sc0.07i -G0 >> %ps%
echo 0.5 0.4 Satellite | gmt pstext -R -Jx -F+f14p,Helvetica-Bold+jLM -O >> %ps%
REM
del box.d
del *.pg
del spectrum.*
del samp.x
del .gmt*
