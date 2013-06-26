@echo off

REM	$Id$
REM
REM call fromExamples 40 %name% %ext% %frmt% %path_build% %pathGallery%
REM %1 -> example number
REM %2 -> %name%
REM %3 -> %ext%
REM %4 -> %frmt%
REM %5 -> %path_build%
REM %6 -> %pathGallery%

IF "%3"=="pdf" (
	REM gmt ps2raster %~dp0../examples/ex%1/example_%1.ps -A+S0.6 -P -T%4 -D%5
	echo .. figure:: %5/%2.%3 > %6\fig_%2.rst_
	echo.   >> %6\fig_%2.rst_

	echo .. ^|ex%1^| image:: %5/%2.%3 > %6\img_%2.rst_
	echo    :width: 150 px >> %6\img_%2.rst_
) ELSE (
	REM gmt ps2raster %~dp0../examples/ex%1/example_%1.ps -A -E150 -P -T%frmt% -Qt4 -D%5
	echo .. figure:: %5/%2.%3 > %6\fig_%2.rst_
	echo    :width: 500 px >> %6\fig_%2.rst_
	echo    :align: center >> %6\fig_%2.rst_
	echo.   >> %6/fig_%2.rst_

	echo .. ^|ex%1^| image:: %5/%2.%3 > %6\img_%2.rst_
	echo    :width: 150 px >> %6\img_%2.rst_
)
