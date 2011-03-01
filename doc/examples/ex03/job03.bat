REM
REM		GMT EXAMPLE 03
REM
REM		$Id: job03.bat,v 1.14 2011-03-01 01:34:48 remko Exp $
REM
echo GMT EXAMPLE 03
set master=y
if exist job03.bat set master=n
if %master%==y cd ex03
REM This version of example 3 is short and ugly.  We simply do not
REM have access to all the UNIX tools needed. See job03.sh for details.
REM First, we use "fitcircle" to find the parameters of a great circle
REM most closely fitting the x,y points in "sat.xyg":
REM We find that center is 330.169/-18.4207 and pole is 52.7452/21.204
REM
REM Now we use "project" to project the data in both sat.xyg and ship.xyg
REM into data.pg, where g is the same and p is the oblique longitude around
REM the great circle.  We use -Q to get the p distance in kilometers, and -S
REM to sort the output into increasing p values.
REM
project  sat.xyg -C330.169/-18.4207 -T52.7452/21.204 -S -Fpz -Q > sat.pg
project ship.xyg -C330.169/-18.4207 -T52.7452/21.204 -S -Fpz -Q > ship.pg
REM Now we can use sampr in gawk to make a sampling points file for sample1d:
gmtmath -T-1167/1169/1 -N1/0 = samp.x
REM
REM Now we can resample the projected satellite data:
REM
sample1d sat.pg -Nsamp.x > samp_sat.pg
REM
REM For reasons above, we use filter1d to pre-treat the ship data.  We also need to sample it
REM because of the gaps over 1 km we found.  So we use filter1d and sample1d.  We also use the -E
REM on filter1d to use the data all the way out to sampr1/sampr2 :
REM
filter1d ship.pg -Fm1 -T-1167/1169/1 -E | sample1d -Nsamp.x > samp_ship.pg
REM Now to do the cross-spectra, assuming that the ship is the input and the sat is the output 
REM data, we do this:
REM 
spectrum1d -S256 -D1 -W -C shipsat.dos
REM 
REM Now we want to plot the spectra.  The following commands will plot the ship and sat 
REM power in one diagram and the coherency on another diagram,  both on the same page.  
REM Note the extended use of pstext and psxy to put labels and legends directly on the plots.  
REM For that purpose we often use -Jx1i and specify positions in inches directly:
REM
psxy spectrum.coh -Ba1f3p:"Wavelength (km)":/a0.25f0.05:"Coherency@+2@+":WeSn -JX-4il/3.75i -R1/1000/0/1 -U/-2.25i/-1.25i/"Example 3 in Cookbook" -P -K -X2.5i -Sc0.07i -G0 -Ey/0.5p -Y1.5i > ..\example_03.ps
echo 3.85 3.6 18 0.0 1 11 Coherency@+2@+ | pstext -R0/4/0/3.75 -Jx1i -O -K >> ..\example_03.ps
echo 2.375 3.75 > box.d
echo 2.375 3.25 >> box.d
echo 4 3.25 >> box.d
psxy -R -J -O -K -Wthicker box.d >> ..\example_03.ps
psxy -Gblack -ST0.07i -O -Ba1f3p/a1f3p:"Power (mGal@+2@+km)"::."Ship and Satellite Gravity":WeSn spectrum.xpower -R1/1000/0.1/10000 -JX-4il/3.75il -Y4.2i -K -Ey/0.5p >> ..\example_03.ps
psxy spectrum.ypower -R -JX -O -K -G0 -Sc0.07i -Ey/0.5p >> ..\example_03.ps
echo 3.9 3.6 18 0.0 1 11 Input Power | pstext -R0/4/0/3.75 -Jx1 -O -K >> ..\example_03.ps
psxy -R -J -O -K -Wthicker box.d >> ..\example_03.ps
echo 0.25 0.25 > box.d
echo 1.4 0.25 >> box.d
echo 1.4 0.9 >> box.d
echo 0.25 0.9 >> box.d
psxy -R -J -O -K -G240 -L -Wthicker box.d >> ..\example_03.ps
echo 0.4 0.7 | psxy -R -J -O -K -ST0.07i -G0 >> ..\example_03.ps
echo 0.5 0.7 14 0.0 1 5 Ship | pstext -R -J -O -K >> ..\example_03.ps
echo 0.4 0.4 | psxy -R -J -O -K -Sc0.07i -G0 >> ..\example_03.ps
echo 0.5 0.4 14 0.0 1 5 Satellite | pstext -R -J -O >> ..\example_03.ps
REM
del box.d
del *.pg
del spectrum.*
del samp.x
del .gmt*
if %master%==y cd ..
