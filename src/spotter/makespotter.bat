ECHO OFF
REM ----------------------------------------------------
REM
REM	$Id: makespotter.bat,v 1.9 2011-04-23 02:14:13 guru Exp $
REM
REM
REM	Copyright (c) 1991-2010 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
REM	See LICENSE.TXT file for copying and redistribution conditions.
REM
REM	This program is free software; you can redistribute it and/or modify
REM	it under the terms of the GNU General Public License as published by
REM	the Free Software Foundation; version 2 or any later version.
REM
REM	This program is distributed in the hope that it will be useful,
REM	but WITHOUT ANY WARRANTY; without even the implied warranty of
REM	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
REM	GNU General Public License for more details.
REM
REM	Contact info: www.soest.hawaii.edu/gmt
REM --------------------------------------------------------------------
REM This extremely lame DOS batch file will compile
REM the GMT supplemental package SPOTTER under WIN32 using
REM Microsoft Visual C/C++ tools.
REM
REM Author: Paul Wessel, 18-OCT-2007
REM ----------------------------------------------------
REM
REM How to make and install SPOTTER under Win95/98/NT:
REM
REM STEP a: Install GMT using instructions in gmtinstall.bat
REM
REM STEP b: If you DID NOT install netcdf as a DLL you must
REM	    remove REM from the 2nd DLL line below:
SET DLL_NETCDF="/DDLL_NETCDF"
REM SET DLL_NETCDF=
REM
REM STEP c: Change BINDIR if necessary
SET BINDIR="..\..\bin"
REM
REM ----------------------------------------------------
REM STOP HERE - THE REST IS AUTOMATIC
REM ----------------------------------------------------
set COPT=/I.. /DWIN32 %SI% %DLL_NETCDF% /DDLL_PSL /DDLL_GMT /W3 /O2 /nologo
set LIBS=%BINDIR%\gmt.lib %BINDIR%\psl.lib netcdf.lib
ECHO STEP 1: Make spotter library
REM ----------------------------------------------------
CL %COPT% /c libspotter.c
LIB /OUT:spotter.lib libspotter.obj
REM ----------------------------------------------------
ECHO STEP 2: Compile and link programs
REM ----------------------------------------------------
CL %COPT% backtracker.c spotter.lib %LIBS%
CL %COPT% grdrotater.c  spotter.lib %LIBS%
CL %COPT% grdspotter.c  spotter.lib %LIBS%
CL %COPT% hotspotter.c  spotter.lib %LIBS%
CL %COPT% originator.c  spotter.lib %LIBS%
CL %COPT% rotconverter.c  spotter.lib %LIBS%
del *.obj
move *.exe %BINDIR%
