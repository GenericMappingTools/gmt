REM
REM Run some simple GMT commands
REM

REM Check GMT splash screen
gmt

REM Check GMT configuration
bash %INSTALLDIR%/bin/gmt-config --all

REM Check GMT defaults
gmt defaults -Vd

REM Check GMT classic mode, GSHHG and DCW
gmt pscoast -R0/10/0/10 -JM6i -Ba -Ggray -ENG+p1p,blue -P -Vd > test.ps

REM Check GMT modern mode, GSHHG and DCW
gmt begin && gmt coast -R0/10/0/10 -JM6i -Ba -Ggray -ENG+p1p,blue -Vd && gmt end

REM Check remote file and modern one-liner
REM gmt grdimage @earth_relief_01d -JH10c -Baf -pdf map

REM Check supplemental modules
gmt earthtide -T2018-06-18T12:00:00 -Gsolid_tide_up.grd

REM Check OpenMP support
REM gmt grdsample @earth_relief_01d -R0/20/0/20 -I30m -Gtopo_30m.nc -x2
gmt grdlandmask -R-60/-40/-40/-30 -I5m -N1/NaN -Gland_mask.nc -x2
