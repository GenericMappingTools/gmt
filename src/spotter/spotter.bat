echo OFF
REM
REM
REM	Examples of how to use the SPOTTER package
REM	DOS version
REM
REM	Paul Wessel
REM	29-OCT-2015
REM

REM Rotation poles to use
set POLES=WK97.txt
REM set POLES=DC85.txt

REM Example 1 - Using gmt backtracker
REM
REM We will use gmt backtracker to test all four functions.  We will
REM 1. Plot hotspot track from Loihi forwards for 80 m.y.
REM 2. forthtrack where Loihi will be in 80 m.y
REM 3. Plot flowline from Suiko back until paleoridge (100 Ma)
REM 4. Backtrack the location of Suiko using an age of 64.7 Ma

echo Running example 1...

echo 205 20 80.0 > loihi.txt
echo 170 44 100 > suiko.txt
gmt pscoast -R150/220/00/65 -JM6i -P -K -Gdarkgreen -A500 -Dl -W0.25p -Baf -BWSne > example_1.ps
gmt psxy -R -JM -O -K -SC0.1i -Gred -W0.5p loihi.txt >> example_1.ps
REM Task 1.1:
gmt backtracker loihi.txt -Df -Lb25 -E%POLES% | gmt psxy -R -J -O -K -W1p >> example_1.ps
REM Task 1.2:
gmt backtracker loihi.txt -Df -E%POLES% | gmt psxy -R -J -O -K -SC0.1i -Ggreen -W0.5p >> example_1.ps
REM Task 1.3:
gmt backtracker suiko.txt -Db -Lf25 -E%POLES% | gmt psxy -R -J -O -K -W1p,. >> example_1.ps
echo 170 44 64.7 > suiko.txt
REM Task 1.4:
gmt backtracker suiko.txt -Db -E%POLES% | gmt psxy -R -J -O -K -ST0.1i -Gyellow -W0.5p >> example_1.ps
gmt psxy -R -J -O -ST0.1i -Gcyan -W0.5p suiko.txt >> example_1.ps
echo Done.  View example_1.ps
REM gsview32 example_1.ps

REM Example 2 - Using gmt hotspotter
REM
REM We will use gmt hotspotter to create a CVA image for the Pacific.
REM It will look similar to the ones we have published but we will
REM here use only seamounts with a VGG amplitude of at least 100 Eotvos.

echo Running example 2...

REM The data to use:
set DATA=seamounts.txt
REM Upper age limit:
set tmax=145
REM The grid spacing to use:
set dx=10m
REM Our Pacific region:
set region=130/260/-66/60

gmt hotspotter %DATA% -h -I%dx% -R%region% -E%POLES% -Gexample_1.grd -V -T -N%tmax%

REM Make a suitable color table

gmt makecpt -Chot -T0/3000/300 -Z > t.cpt

gmt grdimage example_1.grd -JM6i -P -K -Ct.cpt -V > example_2.ps
gmt pscoast -R -J -O -Gdarkgreen -A500 -Dl -W0.25p -Baf -BWSne >> example_2.ps
del t.cpt
del loihi.txt
del suiko.txt
echo Done.  View example_2.ps
REM gsview32 example_2.ps

REM Example 3 - Using gmt originater
REM
REM We will use gmt originater to determine the most likely hotspot origins
REM for the seamounts in the seamounts.txt file, given a plate motion model
REM and a list of possible hotspots.

echo Running example 3...

REM The data to use:
set DATA=seamounts.txt
REM The hotspot location file to use:
set HS=pac_hs.txt
REM The flowline sampling interval to use
set dx=10m
REM Number of likely hotspots per seamount to return:
set N=2

gmt originater %DATA% -S%N% -h -D%dx% -E%POLES% -F%HS% -V > example_3.txt

echo Done.  Inspect example_3.txt data file

echo on
