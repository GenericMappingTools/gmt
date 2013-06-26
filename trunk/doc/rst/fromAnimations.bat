@echo off

REM	$Id$
REM
REM call fromAnimations 02 %name% %ext% %frmt% %path_build% %pathGallery%
REM %1 -> animation number
REM %2 -> %name%
REM %3 -> %ext%
REM %4 -> %frmt%
REM %5 -> %path_build%
REM %6 -> %pathGallery%

IF "%3"=="pdf" (
	gmt ps2raster %~dp0..\examples\anim%1\anim_%1.ps -A+S0.6 -P -T%4 -D%5
	echo .. figure:: %5/%2.%3 > %7/fig_%2.rst_
) ELSE (
	gmt ps2raster %~dp0..\examples\anim%1\anim_%1.ps -A -E150 -P -T%frmt% -Qt4 -D%5
	echo .. figure:: %5/%2.%3 > %6/fig_%2.rst_
	echo    :width: 400 px >> %6/fig_%2.rst_
	echo    :align: center >> %6/fig_%2.rst_
	echo.   >> %6/fig_%2.rst_

	echo .. ^|anim%1^| image:: %5/%2.%3 > %6/img_%2.rst_
	echo    :width: 150 px >> %6/img_%2.rst_
)
