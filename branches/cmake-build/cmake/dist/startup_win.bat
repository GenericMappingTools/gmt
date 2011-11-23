@ECHO OFF

REM Startup script for GMT in Windows.

set RUNDIR=%~dp0
set PATH=%PATH%;%RUNDIR%

cd %RUNDIR%\..\share
set GMT_SHAREDIR=%cd%

cd %HOMEDRIVE%
cd %HOMEPATH%

cmd /k gmt
