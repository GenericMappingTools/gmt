echo OFF
REM
REM	$Id$
REM
REM	Examples of how to use the SPOTTER package
REM	DOS version
REM
REM	Paul Wessel
REM	18-OCT-2007
REM

REM Rotation poles to use
set POLES=WK97.d
REM set POLES=DC85.d

REM Example 1 - Using backtracker
REM
REM We will use backtracker to test all four functions.  We will
REM 1. Plot hotspot track from Loihi forwards for 80 m.y.
REM 2. forthtrack where Loihi will be in 80 m.y
REM 3. Plot flowline from Suiko back until paleoridge (100 Ma)
REM 4. Backtrack the location of Suiko using an age of 64.7 Ma

echo Running example 1...

echo 205 20 80.0 > loihi.d
echo 170 44 100 > suiko.d
pscoast -R150/220/00/65 -JM6i -P -K -G30/120/30 -A500 -Dl -W0.25p -B20WSne > example_1.ps
psxy -R -JM -O -K -SC0.1 -G255/0/0 -W0.5p loihi.d >> example_1.ps
REM Task 1.1:
backtracker loihi.d -Df -Lb25 -E%POLES% | psxy -R -JM -O -K -M -W1p >> example_1.ps
REM Task 1.2:
backtracker loihi.d -Df -E%POLES% | psxy -R -JM -O -K -SC0.1 -G0/255/0 -W0.5p >> example_1.ps
REM Task 1.3:
backtracker suiko.d -Db -Lf25 -E%POLES% | psxy -R -JM -O -K -M -W1top >> example_1.ps
echo 170 44 64.7 > suiko.d
REM Task 1.4:
backtracker suiko.d -Db -E%POLES% | psxy -R -JM -O -K -ST0.1 -G255/255/0 -W0.5p >> example_1.ps
psxy -R -JM -O -ST0.1 -G0/255/255 -W0.5p suiko.d >> example_1.ps
echo Done.  View example_1.ps
REM gsview32 example_1.ps

REM Example 2 - Using hotspotter
REM
REM We will use hotspotter to create a CVA image for the Pacific.
REM It will look similar to the ones we have published but we will
REM here use only seamounts with a VGG amplitude of at least 100 Eotvos.

echo Running example 2...

REM The data to use:
set DATA=seamounts.d
REM Upper age limit:
set tmax=145
REM The grid spacing to use:
set dx=10m
REM Our Pacific region:
set region=130/260/-66/60

hotspotter %DATA% -h -I%dx% -R%region% -E%POLES% -Gexample_1.grd -V -T -N%tmax%

REM Make a suitable color table

makecpt -Chot -T0/3000/300 -Z > t.cpt

grdimage example_1.grd -JM6i -P -K -Ct.cpt -V > example_2.ps
pscoast -R -JM -O -G30/120/30 -A500 -Dl -W0.25p -B20WSne >> example_2.ps
del t.cpt
del loihi.d
del suiko.d
echo Done.  View example_2.ps
REM gsview32 example_2.ps

REM Example 3 - Using originator
REM
REM We will use originator to determine the most likely hotspot origins
REM for the seamounts in the seamounts.d file, given a plate motion model
REM and a list of possible hotspots.

echo Running example 3...

REM The data to use:
set DATA=seamounts.d
REM The hotspot location file to use:
set HS=pac_hs.d
REM The flowline sampling interval to use
set dx=10m
REM Number of likely hotspots per seamount to return:
set N=2

originator %DATA% -S%N% -h -D%dx% -E%POLES% -F%HS% -V > example_3.d

echo Done.  Inspect example_3.d data file

echo on
