ECHO OFF
REM ----------------------------------------------------
REM
REM	$Id: gmtsuppl.bat,v 1.11 2004-01-12 01:37:26 pwessel Exp $
REM
REM
REM	Copyright (c) 1991-2004 by P. Wessel and W. H. F. Smith
REM	See COPYING file for copying and redistribution conditions.
REM
REM	This program is free software; you can redistribute it and/or modify
REM	it under the terms of the GNU General Public License as published by
REM	the Free Software Foundation; version 2 of the License.
REM
REM	This program is distributed in the hope that it will be useful,
REM	but WITHOUT ANY WARRANTY; without even the implied warranty of
REM	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
REM	GNU General Public License for more details.
REM
REM	Contact info: gmt.soest.hawaii.edu
REM --------------------------------------------------------------------
REM This extremely lame DOS batch file will compile
REM the GMT 4 supplemental programs under WIN32 using
REM Microsoft Visual C/C++ tools.  Not yet set up for mex.
REM Note: Optimizing all at /O2 except meca which seems unstable
REM
REM Author: Paul Wessel, 10-JAN-2004
REM ----------------------------------------------------
REM
REM How to make and install GMT under Win95/98/NT/2K/XP:
REM
REM STEP a: Install GMT using instructions in gmtinstall.bat
REM
REM STEP b: If you DID NOT install netcdf as a DLL you must
REM	    change the setting to "no" here:
REM
SET DLLCDF="yes"
REM SET DLLCDF="no"
REM
REM STEP c: Change BINDIR and LIBDIR if necessary
REM
SET BINDIR="..\..\bin"
SET LIBDIR="..\..\lib"
REM
REM STEP d: If you built GMT using static libraries,
REM	    change CHOICE to "static" here:
REM
SET CHOICE="dynamic"
REM SET CHOICE="static"
REM ----------------------------------------------------
REM STOP HERE - THE REST IS AUTOMATIC
REM ----------------------------------------------------
SET DLL_NETCDF="/DDLL_NETCDF"
IF %DLLCDF%=="no" SET DLL_NETCDF=
SET COPT=/I.. /DWIN32 /W3 /O2 /nologo %DLL_NETCDF% /DDLL_PSL /DDLL_GMT 
IF %CHOICE%=="static" SET COPT=/I.. /DWIN32 /W3 /O2 /nologo %DLL_NETCDF%
SET COPT2=/I.. /DWIN32 /W3 /nologo %DLL_NETCDF% /DDLL_PSL /DDLL_GMT 
IF %CHOICE%=="static" SET COPT2=/I.. /DWIN32 /W3 /nologo %DLL_NETCDF%
SET LOPT=/nologo /dll /incremental:no
set GMTLIB=%LIBDIR%\gmt.lib %LIBDIR%\psl.lib netcdf.lib setargv.obj
REM ----------------------------------------------------
ECHO STEP 1: Make dbase
REM ----------------------------------------------------
cd dbase
CL %COPT% grdraster.c %GMTLIB%
del *.obj
move *.exe %BINDIR%
cd ..
REM ----------------------------------------------------
ECHO STEP 2: Make gshhs
REM ----------------------------------------------------
cd gshhs
CL %COPT% gshhs.c %GMTLIB%
CL %COPT% gshhs_dp.c %GMTLIB%
CL %COPT% gshhstograss.c %GMTLIB%
del *.obj
move *.exe %BINDIR%
cd ..
REM ----------------------------------------------------
ECHO STEP 3: Make imgsrc
REM ----------------------------------------------------
cd imgsrc
CL %COPT% img2mercgrd.c gmt_imgsubs.c %GMTLIB%
del *.obj
move *.exe %BINDIR%
cd ..
REM ----------------------------------------------------
ECHO STEP 4: Make meca
REM ----------------------------------------------------
cd meca
CL %COPT2% /c nrutil.c distaz.c utilmeca.c utilstrain.c submeca.c utilvelo.c
CL %COPT2% pscoupe.c utilmeca.obj submeca.obj distaz.obj nrutil.obj %GMTLIB%
CL %COPT2% psmeca.c utilmeca.obj nrutil.obj %GMTLIB%
CL %COPT2% pspolar.c utilmeca.obj submeca.obj distaz.obj nrutil.obj %GMTLIB%
CL %COPT2% psvelo.c utilvelo.obj utilstrain.obj %GMTLIB%
del *.obj
move *.exe %BINDIR%
cd ..
REM ----------------------------------------------------
ECHO STEP 5: Make mex
REM ----------------------------------------------------
echo Follow Matlab instructions on how to make mex files
REM ----------------------------------------------------
ECHO STEP 6: Make mgg
REM ----------------------------------------------------
cd mgg
IF %CHOICE%=="dynamic" CL %COPT% %DLL_NETCDF% /FD /ML /DDLL_EXPORT /c gmt_mgg.c
IF %CHOICE%=="dynamic" LINK %LOPT% /out:gmt_mgg.dll /implib:gmt_mgg.lib gmt_mgg.obj %GMTLIB%
IF %CHOICE%=="static"  CL %COPT% %DLL_NETCDF% /DDLL_EXPORT /c gmt_mgg.c
IF %CHOICE%=="static"  lib /out:gmt_mgg.lib gmt_mgg.obj
CL %COPT% binlegs.c    gmt_mgg.lib %GMTLIB%
CL %COPT% gmt2bin.c    gmt_mgg.lib %GMTLIB%
CL %COPT% gmt2dat.c    gmt_mgg.lib %GMTLIB%
CL %COPT% dat2gmt.c    gmt_mgg.lib %GMTLIB%
CL %COPT% gmtinfo.c    gmt_mgg.lib %GMTLIB%
CL %COPT% gmtlegs.c    gmt_mgg.lib %GMTLIB%
CL %COPT% gmtlist.c    gmt_mgg.lib %GMTLIB%
CL %COPT% gmtpath.c    gmt_mgg.lib %GMTLIB%
CL %COPT% gmttrack.c   gmt_mgg.lib %GMTLIB%
CL %COPT% mgd77togmt.c gmt_mgg.lib %GMTLIB%
del *.obj
IF %CHOICE%=="dynamic" move gmt_mgg.dll %BINDIR%
IF %CHOICE%=="dynamic" move gmt_mgg.exp %LIBDIR%
move gmt_mgg.lib %LIBDIR%
move *.exe %BINDIR%
cd ..
REM ----------------------------------------------------
ECHO STEP 7: Make misc
REM ----------------------------------------------------
cd misc
SET DIG="\"COM0:\""
CL %COPT% psmegaplot.c %GMTLIB%
CL %COPT% makepattern.c %GMTLIB%
del *.obj
move *.exe %BINDIR%
cd ..
REM ----------------------------------------------------
ECHO STEP 8: Make segyprogs
REM ----------------------------------------------------
cd segyprogs
CL %COPT% /c segy_io.c
LIB /OUT:segy_io.lib segy_io.obj
CL %COPT% pssegy.c  segy_io.lib %GMTLIB%
CL %COPT% pssegyz.c segy_io.lib %GMTLIB%
CL %COPT% segy2grd.c segy_io.lib %GMTLIB%
del *.obj
move segy_io.lib %LIBDIR%
move *.exe %BINDIR%
cd ..
REM ----------------------------------------------------
ECHO STEP 9: Make spotter
REM ----------------------------------------------------
cd spotter
IF %CHOICE%=="dynamic" CL %COPT% %DLL_NETCDF% /FD /ML /DDLL_EXPORT /c libspotter.c
IF %CHOICE%=="dynamic" LINK %LOPT% /out:spotter.dll /implib:spotter.lib libspotter.obj %GMTLIB%
IF %CHOICE%=="static"  CL %COPT% %DLL_NETCDF% /DDLL_EXPORT /c libspotter.c
IF %CHOICE%=="static"  lib /out:spotter.lib libspotter.obj
CL %COPT% backtracker.c   spotter.lib %GMTLIB%
CL %COPT% hotspotter.c    spotter.lib %GMTLIB%
CL %COPT% originator.c    spotter.lib %GMTLIB%
CL %COPT% rotconverter.c  spotter.lib %GMTLIB%
del *.obj
IF %CHOICE%=="dynamic" move spotter.dll %BINDIR%
IF %CHOICE%=="dynamic" move spotter.exp %LIBDIR%
move spotter.lib %LIBDIR%
move *.exe %BINDIR%
cd ..
REM ----------------------------------------------------
ECHO STEP 10: Make x2sys
REM ----------------------------------------------------
cd x2sys
IF %CHOICE%=="dynamic" CL %COPT% /I..\mgg %DLL_NETCDF% /FD /ML /DDLL_EXPORT /c x2sys.c
IF %CHOICE%=="dynamic" LINK %LOPT% /out:x2sys.dll /implib:x2sys.lib x2sys.obj %LIBDIR%\gmt_mgg.lib %GMTLIB%
IF %CHOICE%=="static"  CL %COPT% /I..\mgg %DLL_NETCDF% /DDLL_EXPORT /c x2sys.c
IF %CHOICE%=="static"  lib /out:x2sys.lib x2sys.obj
CL %COPT% /I..\mgg x2sys_cross.c x2sys.lib %LIBDIR%\gmt_mgg.lib %GMTLIB%
CL %COPT% /I..\mgg x2sys_datalist.c  x2sys.lib %LIBDIR%\gmt_mgg.lib %GMTLIB%
del *.obj
IF %CHOICE%=="dynamic" move x2sys.dll %BINDIR%
IF %CHOICE%=="dynamic" move x2sys.exp %LIBDIR%
move x2sys.lib %LIBDIR%
move *.exe %BINDIR%
cd ..
REM ----------------------------------------------------
ECHO STEP 11: Make x_system
REM ----------------------------------------------------
cd x_system
SET COPT2=%COPT% /I..\mgg
SET GMTLIB2=%LIBDIR%\gmt_mgg.lib %GMTLIB%
CL %COPT2% x_edit.c %GMTLIB2%
CL %COPT2% x_init.c %GMTLIB2%
CL %COPT2% x_list.c %GMTLIB2%
CL %COPT2% x_over.c %GMTLIB2%
CL %COPT2% x_remove.c %GMTLIB2%
CL %COPT2% x_report.c %GMTLIB2%
CL %COPT2% x_setup.c %GMTLIB2%
CL %COPT2% x_solve_dc_drift.c %GMTLIB2%
CL %COPT2% x_update.c %GMTLIB2%
del *.obj
move *.exe %BINDIR%
cd ..
