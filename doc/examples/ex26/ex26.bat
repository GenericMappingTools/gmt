REM		GMT EXAMPLE 26
REM
REM Purpose:	Demonstrate general vertical perspective projection
REM GMT modules:	coast
REM
gmt begin ex26
	REM first do an overhead of the east coast from 160 km altitude point straight down
	set latitude=41.5
	set longitude=-74.0
	set altitude=160.0
	set tilt=0
	set azimuth=0
	set twist=0
	set Width=0.0
	set Height=0.0

	set PROJ=-JG%longitude%/%latitude%/%altitude%/%azimuth%/%tilt%/%twist%/%Width%/%Height%/10c
	gmt coast -Rg %PROJ% -B5g5 -Glightbrown -Slightblue -W -Dl -N1/1p,red -N2/0.5p -Y12c

	REM now point from an altitude of 160 km with a specific tilt and azimuth and with a wider restricted
	REM view and a boresight twist of 45 degrees
	set tilt=55
	set azimuth=210
	set twist=45
	set Width=30.0
	set Height=30.0

	set PROJ=-JG%longitude%/%latitude%/%altitude%/%azimuth%/%tilt%/%twist%/%Width%/%Height%/12c
	gmt coast %PROJ% -B5g5 -Glightbrown -Slightblue -W -Ia/blue -Di -Na -X2.5c -Y-10c
gmt end show
