@ECHO OFF

REM Startup script for GMT in Windows.

set RUNDIR=%~dp0
set PATH=%RUNDIR%;%PATH%

cd %HOMEDRIVE%
cd %HOMEPATH%

cmd /k gmt
