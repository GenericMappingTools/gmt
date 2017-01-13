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
gmt math -T-1167/1169/1 -N1/0 = samp.x
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
gmt spectrum1d -S256 -D1 -W -C shipsat.dos -T
REM 
REM Now we want to plot the spectra.  The following commands will plot the ship and sat 
REM power in one diagram and the coherency on another diagram,  both on the same page.  
REM We end by adding a map legends and some labels on the plots.
REM For that purpose we often use -Jx1i and specify positions in inches directly:
REM
gmt psxy spectrum.coh -Bxa1f3p+l"Wavelength (km)" -Bya0.25f0.05+l"Coherency@+2@+" -BWeSn+g240/255/240 -JX-4il/3.75i -R1/1000/0/1 -P -K -X2.5i -Sc0.07i -Gpurple -Ey+p0.5p -Y1.5i > %ps%
echo Coherency@+2@+ | gmt pstext -R -J -F+cTR+f18p,Helvetica-Bold -Dj0.1i -O -K -Wthicker -C0.1i >> %ps%
gmt psxy -Gred -ST0.07i -O -Bxa1f3p -Bya1f3p+l"Power (mGal@+2@+km)" -BWeSn+t"Ship and Satellite Gravity"+g240/255/240 spectrum.xpower -R1/1000/0.1/10000 -JX-4il/3.75il -Y4.2i -K -Ey+p0.5p >> %ps%
gmt psxy spectrum.ypower -R -JX -O -K -Gblue -Sc0.07i -Ey+p0.5p >> %ps%
echo Input Power | gmt pstext -R -Jx1i -F+cTR+f18p,Helvetica-Bold -Dj0.1i -O -K -Wthicker -C0.1i >> %ps%
echo S 0.1i T 0.07i black - 0.3i Ship > tmp
echo S 0.1i c 0.07i black - 0.3i Satellite >> tmp
gmt pslegend -R0/4/0/3.75 -Jx -O -DjBL+w1.2i+o0.25i -F+glightgray+pthicker --FONT_ANNOT_PRIMARY=14p,Helvetica-Bold tmp >> %ps%
REM
del tmp
del *.pg
del spectrum.*
del samp.x
del .gmt*
