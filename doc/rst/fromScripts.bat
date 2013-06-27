@echo off

REM	$Id$
REM
REM %1 -> %name%
REM %2 -> %ext%
REM %3 -> %frmt%
REM %4 -> %path_build%
REM %5 -> %pato%

IF "%2"=="pdf" (
	gmt ps2raster %~dp0..\scripts\%1.ps -A -P -T%3 -D%4
) ELSE (
	gmt ps2raster %~dp0..\scripts\%1.ps -A -P -T%3 -D%4 -E150 -Qt4
)
echo .. figure:: %4/%1.%2 > %5\fig_%1.rst_
echo    :width: 500 px >> %5\fig_%1.rst_
echo    :align: center >> %5\fig_%1.rst_
echo.   >> %5\fig_%1.rst_
