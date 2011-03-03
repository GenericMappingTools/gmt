ECHO OFF
REM ----------------------------------------------------
REM
REM	$Id: gmtsuppl.bat,v 1.55 2011-03-03 21:02:51 guru Exp $
REM
REM
REM	Copyright (c) 1991-2011 by P. Wessel and W. H. F. Smith
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
REM	Contact info: gmt.soest.hawaii.edu
REM --------------------------------------------------------------------
REM This extremely lame DOS batch file will compile
REM the GMT 4 supplemental programs under WIN32 using
REM Microsoft Visual C/C++ tools.  Not yet set up for mex.
REM Note: Optimizing all at /O2 except meca which seems unstable
REM
REM Author: Paul Wessel, 15-JAN-2010
REM ----------------------------------------------------
REM
REM How to build GMT under Windows:
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
REM
REM STEP e: Specify your compiler (currently set to MS CL)
SET CC=CL
REM SET CC=icl
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
%CC% %COPT% grdraster.c %GMTLIB%
del *.obj
move *.exe %BINDIR%
cd ..
REM ----------------------------------------------------
ECHO STEP 2: Make gshhs
REM ----------------------------------------------------
cd gshhs
%CC% %COPT% gshhs.c %GMTLIB%
%CC% %COPT% gshhs_dp.c %GMTLIB%
%CC% %COPT% gshhstograss.c %GMTLIB%
del *.obj
move *.exe %BINDIR%
cd ..
REM ----------------------------------------------------
ECHO STEP 3: Make imgsrc
REM ----------------------------------------------------
cd imgsrc
%CC% %COPT% img2mercgrd.c gmt_imgsubs.c %GMTLIB%
del *.obj
move *.exe %BINDIR%
cd ..
REM ----------------------------------------------------
ECHO STEP 4: Make meca
REM ----------------------------------------------------
cd meca
%CC% %COPT2% /c nrutil.c distaz.c utilmeca.c utilstrain.c submeca.c utilvelo.c
%CC% %COPT2% pscoupe.c utilmeca.obj submeca.obj distaz.obj nrutil.obj %GMTLIB%
%CC% %COPT2% psmeca.c utilmeca.obj nrutil.obj %GMTLIB%
%CC% %COPT2% pspolar.c utilmeca.obj submeca.obj distaz.obj nrutil.obj %GMTLIB%
%CC% %COPT2% psvelo.c utilvelo.obj utilstrain.obj %GMTLIB%
del *.obj
move *.exe %BINDIR%
cd ..
REM ----------------------------------------------------
ECHO STEP 5: Make mex
REM ----------------------------------------------------
echo Follow Matlab instructions on how to make mex files
REM ----------------------------------------------------
ECHO STEP 6: Make mgd77
REM ----------------------------------------------------
cd mgd77
%CC% %COPT2% /c mgd77.c
lib /out:mgd77.lib mgd77.obj
%CC% %COPT% mgd77convert.c mgd77.lib %GMTLIB%
%CC% %COPT% mgd77info.c mgd77.lib %GMTLIB%
%CC% %COPT% mgd77list.c mgd77.lib %GMTLIB%
%CC% %COPT% mgd77manage.c mgd77.lib %GMTLIB%
%CC% %COPT% mgd77path.c mgd77.lib %GMTLIB%
%CC% %COPT% mgd77sniffer.c mgd77.lib %GMTLIB%
%CC% %COPT% mgd77track.c mgd77.lib %GMTLIB%
%CC% %COPT% mgd77magref.c mgd77.lib %GMTLIB%
del *.obj
move mgd77.lib %LIBDIR%
move *.exe %BINDIR%
cd ..
REM ----------------------------------------------------
ECHO STEP 7: Make mgg
REM ----------------------------------------------------
cd mgg
IF %CHOICE%=="dynamic" %CC% %COPT% %DLL_NETCDF% /FD /DDLL_EXPORT /c gmt_mgg.c
IF %CHOICE%=="dynamic" LINK %LOPT% /out:gmt_mgg.dll /implib:gmt_mgg.lib gmt_mgg.obj %GMTLIB%
IF %CHOICE%=="static"  %CC% %COPT% %DLL_NETCDF% /DDLL_EXPORT /c gmt_mgg.c
IF %CHOICE%=="static"  lib /out:gmt_mgg.lib gmt_mgg.obj
%CC% %COPT% binlegs.c    gmt_mgg.lib %GMTLIB%
%CC% %COPT% gmt2bin.c    gmt_mgg.lib %GMTLIB%
%CC% %COPT% gmt2dat.c    gmt_mgg.lib %GMTLIB%
%CC% %COPT% dat2gmt.c    gmt_mgg.lib %GMTLIB%
%CC% %COPT% gmtinfo.c    gmt_mgg.lib %GMTLIB%
%CC% %COPT% gmtlegs.c    gmt_mgg.lib %GMTLIB%
%CC% %COPT% /I..\x_system gmtlist.c    gmt_mgg.lib %GMTLIB%
%CC% %COPT% gmtpath.c    gmt_mgg.lib %GMTLIB%
%CC% %COPT% gmttrack.c   gmt_mgg.lib %GMTLIB%
%CC% %COPT% mgd77togmt.c gmt_mgg.lib %GMTLIB%
del *.obj
IF %CHOICE%=="dynamic" move gmt_mgg.dll %BINDIR%
IF %CHOICE%=="dynamic" move gmt_mgg.exp %LIBDIR%
move gmt_mgg.lib %LIBDIR%
move *.exe %BINDIR%
cd ..
REM ----------------------------------------------------
ECHO STEP 8: Make misc
REM ----------------------------------------------------
cd misc
REM %CC% %COPT% gmtdigitize.c %GMTLIB%
%CC% %COPT% dimfilter.c %GMTLIB%
%CC% %COPT% gmt2kml.c %GMTLIB%
%CC% %COPT% kml2gmt.c %GMTLIB%
%CC% %COPT% gmtstitch.c %GMTLIB%
%CC% %COPT% psmegaplot.c %GMTLIB%
%CC% %COPT% makepattern.c %GMTLIB%
%CC% %COPT% nc2xy.c %GMTLIB%
del *.obj
move *.exe %BINDIR%
cd ..
REM ----------------------------------------------------
ECHO STEP 9: Make segyprogs
REM ----------------------------------------------------
cd segyprogs
%CC% %COPT% /c segy_io.c
LIB /OUT:segy_io.lib segy_io.obj
%CC% %COPT% pssegy.c  segy_io.lib %GMTLIB%
%CC% %COPT% pssegyz.c segy_io.lib %GMTLIB%
%CC% %COPT% segy2grd.c segy_io.lib %GMTLIB%
del *.obj
move segy_io.lib %LIBDIR%
move *.exe %BINDIR%
cd ..
REM ----------------------------------------------------
ECHO STEP 10: Make sph
REM ----------------------------------------------------
cd sph
IF %CHOICE%=="dynamic" %CC% %COPT% %DLL_NETCDF% /FD /DDLL_EXPORT /c sph.c
IF %CHOICE%=="dynamic" LINK %LOPT% /out:sph.dll /implib:sph.lib sph.obj %GMTLIB%
IF %CHOICE%=="static"  %CC% %COPT% %DLL_NETCDF% /DDLL_EXPORT /c sph.c
IF %CHOICE%=="static"  lib /out:sph.lib sph.obj
%CC% %COPT% sphtriangulate.c sph.lib %GMTLIB%
%CC% %COPT% sphdistance.c    sph.lib %GMTLIB%
%CC% %COPT% sphinterpolate.c sph.lib %GMTLIB%
del *.obj
IF %CHOICE%=="dynamic" move sph.dll %BINDIR%
IF %CHOICE%=="dynamic" move sph.exp %LIBDIR%
move sph.lib %LIBDIR%
move *.exe %BINDIR%
cd ..
REM ----------------------------------------------------
ECHO STEP 11: Make spotter
REM ----------------------------------------------------
cd spotter
IF %CHOICE%=="dynamic" %CC% %COPT% %DLL_NETCDF% /FD /DDLL_EXPORT /c libspotter.c
IF %CHOICE%=="dynamic" LINK %LOPT% /out:spotter.dll /implib:spotter.lib libspotter.obj %GMTLIB%
IF %CHOICE%=="static"  %CC% %COPT% %DLL_NETCDF% /DDLL_EXPORT /c libspotter.c
IF %CHOICE%=="static"  lib /out:spotter.lib libspotter.obj
%CC% %COPT% backtracker.c   spotter.lib %GMTLIB%
%CC% %COPT% grdrotater.c    spotter.lib %GMTLIB%
%CC% %COPT% grdspotter.c    spotter.lib %GMTLIB%
%CC% %COPT% hotspotter.c    spotter.lib %GMTLIB%
%CC% %COPT% originator.c    spotter.lib %GMTLIB%
%CC% %COPT% rotconverter.c  spotter.lib %GMTLIB%
del *.obj
IF %CHOICE%=="dynamic" move spotter.dll %BINDIR%
IF %CHOICE%=="dynamic" move spotter.exp %LIBDIR%
move spotter.lib %LIBDIR%
move *.exe %BINDIR%
cd ..
REM ----------------------------------------------------
ECHO STEP 12: Make x2sys
REM ----------------------------------------------------
cd x2sys
%CC% %COPT2% /I..\mgd77 /I..\mgg /c x2sys.c
lib /out:x2sys.lib x2sys.obj
%CC% %COPT% /I..\mgd77 /I..\mgg x2sys_binlist.c  x2sys.lib %LIBDIR%\mgd77.lib %LIBDIR%\gmt_mgg.lib %GMTLIB%
%CC% %COPT% /I..\mgd77 /I..\mgg x2sys_cross.c x2sys.lib %LIBDIR%\mgd77.lib %LIBDIR%\gmt_mgg.lib %GMTLIB%
%CC% %COPT% /I..\mgd77 /I..\mgg x2sys_datalist.c  x2sys.lib %LIBDIR%\mgd77.lib %LIBDIR%\gmt_mgg.lib %GMTLIB%
%CC% %COPT% /I..\mgd77 /I..\mgg x2sys_get.c  x2sys.lib %LIBDIR%\mgd77.lib %LIBDIR%\gmt_mgg.lib %GMTLIB%
%CC% %COPT% /I..\mgd77 /I..\mgg x2sys_init.c  x2sys.lib %LIBDIR%\mgd77.lib %LIBDIR%\gmt_mgg.lib %GMTLIB%
%CC% %COPT% /I..\mgd77 /I..\mgg x2sys_put.c  x2sys.lib %LIBDIR%\mgd77.lib %LIBDIR%\gmt_mgg.lib %GMTLIB%
%CC% %COPT% /I..\mgd77 /I..\mgg x2sys_list.c  x2sys.lib %LIBDIR%\mgd77.lib %LIBDIR%\gmt_mgg.lib %GMTLIB%
%CC% %COPT% /I..\mgd77 /I..\mgg x2sys_report.c  x2sys.lib %LIBDIR%\mgd77.lib %LIBDIR%\gmt_mgg.lib %GMTLIB%
%CC% %COPT% /I..\mgd77 /I..\mgg x2sys_solve.c  x2sys.lib %LIBDIR%\mgd77.lib %LIBDIR%\gmt_mgg.lib %GMTLIB%
%CC% %COPT% x2sys_merge.c  x2sys.lib %GMTLIB%
del *.obj
move x2sys.lib %LIBDIR%
move *.exe %BINDIR%
cd ..
REM ----------------------------------------------------
ECHO STEP 13: Make x_system
REM ----------------------------------------------------
cd x_system
SET COPT2=%COPT% /I..\mgg
SET GMTLIB2=%LIBDIR%\gmt_mgg.lib %GMTLIB%
%CC% %COPT2% x_edit.c %GMTLIB2%
%CC% %COPT2% x_init.c %GMTLIB2%
%CC% %COPT2% x_list.c %GMTLIB2%
%CC% %COPT2% x_over.c %GMTLIB2%
%CC% %COPT2% x_remove.c %GMTLIB2%
%CC% %COPT2% x_report.c %GMTLIB2%
%CC% %COPT2% x_setup.c %GMTLIB2%
%CC% %COPT2% x_solve_dc_drift.c %GMTLIB2%
%CC% %COPT2% x_update.c %GMTLIB2%
del *.obj
move *.exe %BINDIR%
cd ..
