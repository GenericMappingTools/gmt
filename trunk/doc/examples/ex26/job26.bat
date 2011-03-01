REM
REM             GMT EXAMPLE 26
REM
REM             $Id: job26.bat,v 1.7 2011-03-01 01:34:48 remko Exp $
REM
REM Purpose:    Demonstrate general vertical perspective projection
REM GMT progs:  pscoast
REM DOS calls:  del
REM
echo GMT EXAMPLE 26
set master=y
if exist job26.bat set master=n
if %master%==y cd ex26

REM first do an overhead of the east coast from 160 km altitude point straight down

set latitude=41.5
set longitude=-74.0
set altitude=160.0
set tilt=0
set azimuth=0
set twist=0
set Width=0.0
set Height=0.0

set PROJ=-JG%longitude%/%latitude%/%altitude%/%azimuth%/%tilt%/%twist%/%Width%/%Height%/4i

pscoast -Rg %PROJ% -X1i -B5g5/5g5 -Glightbrown -Slightblue -W0.25p -Dl -N1/1p,red -N2,0.5p -P -K -Y5i > ..\example_26.ps

REM now point from an altitude of 160 km with a specific tilt and azimuth and with
REM a wider restricted view and a boresight twist of 45 degrees

set tilt=55
set azimuth=210
set twist=45
set Width=30.0
set Height=30.0

set PROJ=-JG%longitude%/%latitude%/%altitude%/%azimuth%/%tilt%/%twist%/%Width%/%Height%/5i

pscoast -R %PROJ% -B5g5/5g5 -Glightbrown -Slightblue -W0.25p -Ia/blue -Di -Na -O -X1i -Y-4i -U/-1.75i/-0.75i/"Example 26 in Cookbook" >> ..\example_26.ps
if %master%==n echo on
del .gmt*
if %master%==y cd ..
