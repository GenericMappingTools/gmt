ECHO OFF
REM ----------------------------------------------------
REM
REM	$Id: gmtinstall.bat,v 1.14 2004-10-01 20:32:53 pwessel Exp $
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
REM the GMT 4 suite of programs under WIN32 using
REM Microsoft Visual C/c++ tools.  It will build GMT
REM using DLL libraries.  To make static executables
REM you must make some edits to the setup below.
REM
REM Author: Paul Wessel, 10-JAN-2004
REM ----------------------------------------------------
REM
REM How to make and install GMT under DOS/Win95/98:
REM [Under NT/2000/XP you may edit the Environmental
REM  variables from the System settings instead]
REM
REM STEP a: Install netcdf 3.5 (compile it yourself or get
REM	    ready-to-go binaries from www.unidata.ucar.edu
REM	    If you DID NOT install it as a DLL you must
REM	    change the setting to "no" here:
REM
SET DLLCDF="yes"
REM
REM STEP b: Set the environment needed by MSC by running
REM	    C:\MSVC\DEVSTUDIO\VC\BIN\VCVARS32.BAT
REM	    (your MSVC directory may be different)
REM
REM STEP c: Modify GMTENV.BAT.  Later, you may want to run it
REM	    from inside autoexec.bat so GMTHOME and PATH are set.
REM	    Here, we call it directly:
CALL GMTENV.BAT
REM
REM STEP d: Check/edit definitions of BINDIR, LIBDIR, and GMT_PATH.
REM	    Make sure BINDIR below points to a valid directory
REM	    where you want executables to be installed. Either
REM	    edit BINDIR or create the ..\bin directory.
REM	    Same goes for LIBDIR where GMT libraries will be kept.
REM	    GMT_PATH is where GMT expects to find the
REM	    subdir share.  It is ONLY used if the user does not
REM	    set %GMTHOME%.
REM
SET BINDIR="..\bin"
SET LIBDIR="..\lib"
SET GMT_PATH="\"C:\\gmt\""
REM
REM STEP e: If you WANT TO  use Shewchuk's triangulation
REM	    routine, you must set TRIANGLE to "yes" :
REM
SET TRIANGLE="no"
REM
REM STEP f: By default, GMT will be built dynamically  If
REM	    you do NOT want to use Dynamic link libraries
REM	    (DLL) change CHOICE to "static" here:
REM
REM SET "static"
SET CHOICE="dynamic"
REM ----------------------------------------------------
REM STOP HERE - THE REST IS AUTOMATIC
REM ----------------------------------------------------

SET DLL_NETCDF="/DDLL_NETCDF"
IF %DLLCDF%=="no" SET DLL_NETCDF=
SET COPT=/DWIN32 /W3 /O2 /nologo %DLL_NETCDF% /DDLL_PSL /DDLL_GMT
SET DLL=/FD /ML
IF %CHOICE%=="static" SET COPT=/DWIN32 /W3 /O2 /nologo %DLL_NETCDF%
IF %CHOICE%=="static" SET DLL=
set LOPT=/nologo /dll /incremental:no
SET TR=
SET TROBJ=
IF %TRIANGLE%=="yes" SET TR="/DUSE_TRIANGLE"
IF %TRIANGLE%=="yes" SET TROBJ=triangle.obj
REM ----------------------------------------------------
ECHO STEP 1: Make PS library
REM ----------------------------------------------------
CL %COPT% %TR% /c %DLL% /DDLL_EXPORT /DGMT_DEFAULT_PATH=%GMT_PATH% pslib.c
IF %CHOICE%=="dynamic" link %LOPT% /out:psl.dll /implib:psl.lib pslib.obj
IF %CHOICE%=="static" lib /out:psl.lib pslib.obj
REM ----------------------------------------------------
ECHO STEP 2: Make GMT library
REM ----------------------------------------------------
CL %COPT% gmt_nan_init.c netcdf.lib
gmt_nan_init
del gmt_nan_init.obj
del gmt_nan_init.exe
CL %COPT% %TR% /c %DLL% /DDLL_EXPORT /DGMT_DEFAULT_PATH=%GMT_PATH% gmt_cdf.c gmt_customio.c gmt_grdio.c gmt_init.c
CL %COPT% %TR% /c %DLL% /DDLL_EXPORT /DGMT_DEFAULT_PATH=%GMT_PATH% gmt_io.c gmt_map.c gmt_plot.c gmt_shore.c gmt_stat.c
CL %COPT% %TR% /c %DLL% /DDLL_EXPORT /DGMT_DEFAULT_PATH=%GMT_PATH% gmt_calclock.c gmt_support.c gmt_vector.c fourt.c
IF %TRIANGLE%=="yes" CL %COPT% /c /DNO_TIMER /DTRILIBRARY /DREDUCED /DCDT_ONLY triangle.c
IF %CHOICE%=="dynamic" link %LOPT% /out:gmt.dll /implib:gmt.lib gmt_*.obj fourt.obj %TROBJ% psl.lib netcdf.lib setargv.obj
IF %CHOICE%=="static" lib /out:gmt.lib gmt_*.obj fourt.obj %TROBJ%
REM ----------------------------------------------------
ECHO STEP 3: Make GMT programs 
REM ----------------------------------------------------
set GMTLIB=gmt.lib psl.lib netcdf.lib setargv.obj
CL %COPT% blockmean.c %GMTLIB%
CL %COPT% blockmedian.c %GMTLIB%
CL %COPT% blockmode.c %GMTLIB%
CL %COPT% filter1d.c %GMTLIB%
CL %COPT% fitcircle.c %GMTLIB%
CL %COPT% gmt2rgb.c %GMTLIB%
CL %COPT% gmtconvert.c %GMTLIB%
CL %COPT% gmtdefaults.c %GMTLIB%
CL %COPT% gmtmath.c %GMTLIB%
CL %COPT% gmtselect.c %GMTLIB%
CL %COPT% gmtset.c %GMTLIB%
CL %COPT% grdfilter.c %GMTLIB%
CL %COPT% grd2cpt.c %GMTLIB%
CL %COPT% grd2xyz.c %GMTLIB%
CL %COPT% grdblend.c %GMTLIB%
CL %COPT% grdclip.c %GMTLIB%
CL %COPT% grdcontour.c %GMTLIB%
CL %COPT% grdcut.c %GMTLIB%
CL %COPT% grdedit.c %GMTLIB%
CL %COPT% grdfft.c %GMTLIB%
CL %COPT% grdgradient.c %GMTLIB%
CL %COPT% grdhisteq.c %GMTLIB%
CL %COPT% grdimage.c %GMTLIB%
CL %COPT% grdinfo.c %GMTLIB%
CL %COPT% grdlandmask.c %GMTLIB%
CL %COPT% grdmask.c %GMTLIB%
CL %COPT% grdmath.c %GMTLIB%
CL %COPT% grdpaste.c %GMTLIB%
CL %COPT% grdproject.c %GMTLIB%
CL %COPT% grdreformat.c %GMTLIB%
CL %COPT% grdsample.c %GMTLIB%
CL %COPT% grdtrend.c %GMTLIB%
CL %COPT% grdtrack.c %GMTLIB%
CL %COPT% grdvector.c %GMTLIB%
CL %COPT% grdview.c %GMTLIB%
CL %COPT% grdvolume.c %GMTLIB%
CL %COPT% makecpt.c %GMTLIB%
CL %COPT% mapproject.c %GMTLIB%
CL %COPT% minmax.c %GMTLIB%
CL %COPT% nearneighbor.c %GMTLIB%
CL %COPT% project.c %GMTLIB%
CL %COPT% psbasemap.c %GMTLIB%
CL %COPT% psclip.c %GMTLIB%
CL %COPT% pscoast.c %GMTLIB%
CL %COPT% pscontour.c %GMTLIB%
CL %COPT% pshistogram.c %GMTLIB%
CL %COPT% pslegend.c %GMTLIB%
CL %COPT% psimage.c %GMTLIB%
CL %COPT% psmask.c %GMTLIB%
CL %COPT% psrose.c %GMTLIB%
CL %COPT% psscale.c %GMTLIB%
CL %COPT% pstext.c %GMTLIB%
CL %COPT% pswiggle.c %GMTLIB%
CL %COPT% psxy.c %GMTLIB%
CL %COPT% psxyz.c %GMTLIB%
CL %COPT% sample1d.c %GMTLIB%
CL %COPT% spectrum1d.c %GMTLIB%
CL %COPT% splitxyz.c %GMTLIB%
CL %COPT% surface.c %GMTLIB%
CL %COPT% trend1d.c %GMTLIB%
CL %COPT% trend2d.c %GMTLIB%
CL %COPT% triangulate.c %GMTLIB%
CL %COPT% xyz2grd.c %GMTLIB%
REM ----------------------------------------------------
ECHO STEP 4: Clean up and install executables and libraries
REM ----------------------------------------------------
DEL *.obj
MOVE *.exe %BINDIR%
MOVE *.lib %LIBDIR%
IF %CHOICE%=="dynamic" MOVE *.dll %BINDIR%
IF %CHOICE%=="dynamic" MOVE *.exp %LIBDIR%
