@echo off

REM	$Id$
REM
REM %1 -> %name%
REM %2 -> %pato%

REM set W=gmt grdinfo -C ../fig/%1% | gawk "print $3"
REM set H=gmt grdinfo -C ../fig/%1% | gawk "print $5"
REM FOR /F %%I IN ('gmt grdinfo -C ../fig/%1% | gawk "print $3"') DO SET W=%%I
REM FOR /F %%I IN ('gmt grdinfo -C ../fig/%1% | gawk "print $5"') DO SET H=%%I
echo .. figure:: %~dp0../fig/%1 > %2\fig_%1.rst_
REM echo    :height: %H% px >> %2\fig_%1.rst_
REM echo    :width: %W% px >> %2\fig_%1.rst_
echo    :align: center >> %2\fig_%1.rst_
echo    :scale: 50 ^%% >> %2\fig_%1.rst_
echo.   >> %2\fig_%1.rst_
