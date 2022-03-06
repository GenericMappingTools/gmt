REM
REM Run some simple GMT commands
REM

REM Check GMT version
gmt --version

REM Check GMT configuration
bash %INSTALLDIR%/bin/gmt-config --all

REM Check GMT defaults
gmt defaults -Vd

REM Check GMT classic mode, GSHHG and DCW
gmt pscoast -R0/10/0/10 -JM6i -Ba -Ggray -ENG+p1p,blue -P -Vd > test.ps

REM Check GMT modern mode, GSHHG and DCW
gmt begin && gmt coast -R0/10/0/10 -JM6i -Ba -Ggray -ENG+p1p,blue -Vd && gmt end

REM Check remote file and modern one-liner
gmt grdimage @earth_relief_01d -JH10c -Baf -pdf map

REM Check supplemental modules
gmt earthtide -T2018-06-18T12:00:00 -Gsolid_tide_up.grd
