ECHO OFF
REM ----------------------------------------------------
REM
REM	$Id: gmtinstall.bat,v 1.1.1.1 2000-12-28 01:23:45 gmt Exp $
REM
REM
REM	Copyright (c) 1991-2001 by P. Wessel and W. H. F. Smith
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
REM	Contact info: www.soest.hawaii.edu/gmt
REM --------------------------------------------------------------------
REM This extremely lame DOS batch file will compile
REM the GMT 3.3.6 suite of programs under WIN32 using
REM Microsoft Visual C/c++ tools.  It will build GMT
REM using DLL libraries.  To make static executables
REM you must make some edits to the setup below.
REM
REM Author: Paul Wessel, 04-OCT-2000
REM ----------------------------------------------------
REM
REM How to make and install GMT under DOS/Win95/98:
REM [Under NT or 2000 you must edit the Environmental
REM  variables from the System settings instead]
REM
REM STEP a: Install netcdf 3.4 (compile it yourself or get
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
REM STEP c: Modify GMTENV.BAT and run it (you may want to
REM	    do this from inside autoexec.bat)
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
REM STEP e: If you WANT TO  use Shewchuck's triangulation
REM	    routine, you must set TRIANGLE to yes:
REM
SET TRIANGLE=no
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
CL %COPT% %TR% /c %DLL% /DDLL_EXPORT /DGMT_DEFAULT_PATH=%GMT_PATH% gmt_support.c gmt_vector.c fourt.c
IF %TRIANGLE%=="yes" CL %COPT% /c /DNO_TIMER /DTRILIBRARY /DREDUCED /DCDT_ONLY triangle.c
IF %CHOICE%=="dynamic" link %LOPT% /out:gmt.dll /implib:gmt.lib gmt_*.obj fourt.obj %TROBJ% psl.lib netcdf.lib
IF %CHOICE%=="static" lib /out:gmt.lib gmt_*.obj fourt.obj %TROBJ%
REM ----------------------------------------------------
ECHO STEP 3: Make GMT programs 
REM ----------------------------------------------------
set LIBS=gmt.lib psl.lib netcdf.lib
CL %COPT% blockmean.c %LIBS%
CL %COPT% blockmedian.c %LIBS%
CL %COPT% blockmode.c %LIBS%
CL %COPT% filter1d.c %LIBS%
CL %COPT% fitcircle.c %LIBS%
CL %COPT% gmtconvert.c %LIBS%
CL %COPT% gmtdefaults.c %LIBS%
CL %COPT% gmtmath.c %LIBS%
CL %COPT% gmtselect.c %LIBS%
CL %COPT% gmtset.c %LIBS%
CL %COPT% grdfilter.c %LIBS%
CL %COPT% grd2cpt.c %LIBS%
CL %COPT% grd2xyz.c %LIBS%
CL %COPT% grdclip.c %LIBS%
CL %COPT% grdcontour.c %LIBS%
CL %COPT% grdcut.c %LIBS%
CL %COPT% grdedit.c %LIBS%
CL %COPT% grdfft.c %LIBS%
CL %COPT% grdgradient.c %LIBS%
CL %COPT% grdhisteq.c %LIBS%
CL %COPT% grdimage.c %LIBS%
CL %COPT% grdinfo.c %LIBS%
CL %COPT% grdlandmask.c %LIBS%
CL %COPT% grdmask.c %LIBS%
CL %COPT% grdmath.c %LIBS%
CL %COPT% grdpaste.c %LIBS%
CL %COPT% grdproject.c %LIBS%
CL %COPT% grdreformat.c %LIBS%
CL %COPT% grdsample.c %LIBS%
CL %COPT% grdtrend.c %LIBS%
CL %COPT% grdtrack.c %LIBS%
CL %COPT% grdvector.c %LIBS%
CL %COPT% grdview.c %LIBS%
CL %COPT% grdvolume.c %LIBS%
CL %COPT% makecpt.c %LIBS%
CL %COPT% mapproject.c %LIBS%
CL %COPT% minmax.c %LIBS%
CL %COPT% nearneighbor.c %LIBS%
CL %COPT% project.c %LIBS%
CL %COPT% psbasemap.c %LIBS%
CL %COPT% psclip.c %LIBS%
CL %COPT% pscoast.c %LIBS%
CL %COPT% pscontour.c %LIBS%
CL %COPT% pshistogram.c %LIBS%
CL %COPT% psimage.c %LIBS%
CL %COPT% psmask.c %LIBS%
CL %COPT% psrose.c %LIBS%
CL %COPT% psscale.c %LIBS%
CL %COPT% pstext.c %LIBS%
CL %COPT% pswiggle.c %LIBS%
CL %COPT% psxy.c %LIBS%
CL %COPT% psxyz.c %LIBS%
CL %COPT% sample1d.c %LIBS%
CL %COPT% spectrum1d.c %LIBS%
CL %COPT% splitxyz.c %LIBS%
CL %COPT% surface.c %LIBS%
CL %COPT% trend1d.c %LIBS%
CL %COPT% trend2d.c %LIBS%
CL %COPT% triangulate.c %LIBS%
CL %COPT% xyz2grd.c %LIBS%
REM ----------------------------------------------------
ECHO STEP 4: Clean up and install executables and libraries
REM ----------------------------------------------------
DEL *.obj
MOVE *.exe %BINDIR%
MOVE *.lib %LIBDIR%
IF %CHOICE%=="dynamic" MOVE *.dll %BINDIR%
IF %CHOICE%=="dynamic" MOVE *.exp %LIBDIR%
