REM		GMT EXAMPLE 03
REM
REM Purpose:	Resample track data, do spectral analysis, and plot
REM GMT modules:	filter1d, fitcircle, gmtconvert, gmtinfo, project, sample1d
REM 		spectrum1d, plot, subplot, legend, math
REM DOS calls:	echo, set, del
REM
REM This example begins with data files "ship_03.txt" and "sat_03.txt" which
REM are measurements of a quantity "g" (a "gravity anomaly" which is an
REM anomalous increase or decrease in the magnitude of the acceleration
REM of gravity at sea level).  g is measured at a sequence of points "x,y"
REM which in this case are "longitude,latitude".  The "sat_03.txt" data were
REM obtained by a satellite and the sequence of points lies almost along
REM a great circle.  The "ship_03.txt" data were obtained by a ship which
REM tried to follow the satellite's path but deviated from it in places.
REM Thus the two data sets are not measured at the same of points,
REM and we use various GMT tools to facilitate their comparison.
REM

gmt begin ex03
	gmt set GMT_FFT kiss
	REM First, we use "gmt fitcircle" to find the parameters of a great circle
	REM most closely fitting the x,y points in "sat_03.txt":
	gmt fitcircle @sat_03.txt -L2 -Fm --IO_COL_SEPARATOR=/ > cpos.txt
	set /p cpos=<cpos.txt
	gmt fitcircle @sat_03.txt -L2 -Fn --IO_COL_SEPARATOR=/ > ppos.txt
	set /p ppos=<ppos.txt
	REM Now we use "gmt project" to project the data in both sat_03.txt and ship_03.txt
	REM into data.pg, where g is the same and p is the oblique longitude around
	REM the great circle.  We use -Q to get the p distance in kilometers, and -S
	REM to sort the output into increasing p values.
	gmt project  @sat_03.txt -C%cpos% -T%ppos% -S -Fpz -Q > sat.pg
	gmt project @ship_03.txt -C%cpos% -T%ppos% -S -Fpz -Q > ship.pg
	gmt info ship.pg sat.pg -I1 -Af -L -C -i0  --IO_COL_SEPARATOR=/ > bounds.txt
	set /p bounds=<bounds.txt
	REM Now we can use %bounds% in gmt math to make a sampling points file for gmt sample1d:
	gmt math -T%bounds%/1 -N1/0 T = samp.x
	REM Now we can resample the gmt projected satellite data:
	gmt sample1d sat.pg -Tsamp.x > samp_sat.pg
	REM For reasons above, we use gmt filter1d to pre-treat the ship data.  We also need to sample
	REM it because of the gaps > 1 km we found.  So we use gmt filter1d | gmt sample1d.  We also
	REM use the -E on gmt filter1d to use the data all the way out to bounds :
	gmt filter1d ship.pg -Fm1 -T%bounds%/1 -E | gmt sample1d -Tsamp.x > samp_ship.pg
	REM Now to do the cross-spectra, assuming that the ship is the input and the sat is the output
	REM data, we do this:
	gmt convert -A samp_ship.pg samp_sat.pg -o1,3 | gmt spectrum1d -S256 -D1 -W -C -T
	REM Time to plot spectra, use -l to build a legend
	gmt set FONT_TAG 18p,Helvetica-Bold
	gmt subplot begin 2x1 -M0.3c -SCb+l"Wavelength (km)" -T"Ship and Satellite Gravity" -Fs10c -A+jTR+o8p -BWeSn+g240/255/240
		gmt subplot set 0,0 -A"Input Power"
		gmt plot spectrum.xpower -JX-?l/?l -Bxa1f3p -Bya1f3p+l"Power (mGal@+2@+km)" -Gred -ST5p -R1/1000/0.1/10000 -Ey+p0.5p -lShip
		gmt plot spectrum.ypower -Gblue -Sc5p -Ey+p0.5p -lSatellite
		gmt legend -DjBL+o0.5c -F+gwhite+pthicker --FONT_ANNOT_PRIMARY=14p,Helvetica-Bold
		gmt subplot set 1,0 -A"Coherency@+2@+"
		gmt plot spectrum.coh -JX-?l/? -Bxa1f3p -Bya0.25f0.05+l"Coherency@+2@+" -R1/1000/0/1 -Sc5p -Gpurple -Ey+p0.5p
	gmt subplot end
gmt end show
del samp* *.pg spectrum.* cpos.txt ppos.txt legend.txt
