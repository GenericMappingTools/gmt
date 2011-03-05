ECHO OFF
REM ----------------------------------------------------
REM
REM	$Id: gmtinstall.bat,v 1.55 2011-03-05 19:48:58 guru Exp $
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
REM the GMT 4 suite of programs under WIN32 using
REM Microsoft Visual C/c++ tools.  It will build GMT
REM using DLL libraries.  To make static executables
REM you must make some edits to the setup below.
REM
REM Author: Paul Wessel, 01-MAR-2011
REM ----------------------------------------------------
REM
REM How to build GMT executables under Windows:
REM
REM STEP a: Install netcdf 3.6.3 (compile it yourself or get
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
REM STEP c: Set the environment needed by GMT.  These are
REM	    HOME	Your home directory, perhaps C:\
REM	    GMTHOME	Top dir of the GMT installation tree, e.g., C:\GMT
REM	    GMT_SHAREDIR	Place to read GMT run-time files.  If not
REM				set we use %GMTHOME%\share
REM	    INCLUDE	Add the path to the netCDF include file netcdf.h
REM	    LIB		Add the path to the netCDF library netcdf.lib
REM	    PATH	Add the path to the netCDF DLL library netcdf.dll
REM	    PATH	Add the path to the GMT executables (%GMTHOME%\bin)
REM
REM STEP d: Check/edit definitions of BINDIR, LIBDIR, and GMT_SHARE_PATH.
REM	    Make sure BINDIR below points to a valid directory
REM	    where you want executables to be installed. Either
REM	    edit BINDIR or create the ..\bin directory.
REM	    Same goes for LIBDIR where GMT libraries will be kept.
REM	    GMT_SHARE_PATH is where GMT expects to find the shared data.
REM	    It is ONLY used if the user does not set %GMT_SHAREDIR%.
REM
SET GMT_SHARE_PATH="\"C:\\programs\\GMT\\share\""
REM
REM STEP e: If you WANT TO  use Shewchuk's triangulation
REM	    routine, you must set TRIANGLE to "yes" or
REM	    pass yes as 1st argument to this script:
REM
SET TRIANGLE="no"
IF "%1%" == "yes" set TRIANGLE="yes"
REM
REM STEP f: By default, GMT will be built dynamically  If
REM	    you do NOT want to use Dynamic link libraries
REM	    (DLL) change CHOICE to "static" here:
REM

REM
REM STEP g: To optionally link against the GDAL library you must set
REM	    GDAL to "yes" or pass yes as 2nd argument to this script
REM	    Add the GDAL lib and include paths as for netCDF above.
SET GDAL="no"
IF "%2%" == "yes" set GDAL="yes"
REM
REM STEP h: By default we build 32-bit executables. Give 64 as the 3rd
REM	    argument to this script to build using 64-bit libs.
SET BITS=32
IF "%3%" == "64" set BITS=64
REM
SET USE_GDAL=
IF %GDAL%=="yes" SET USE_GDAL=/DUSE_GDAL

REM STEP h: Specify your compiler (currently set to MS CL)
SET CC=CL
REM SET CC=icl

REM SET CHOICE="static"
SET CHOICE="dynamic"
REM ----------------------------------------------------
REM STOP HERE - THE REST IS AUTOMATIC
REM ----------------------------------------------------
SET BINDIR=..\bin%BITS%
SET LIBDIR=..\lib
SET INCDIR=..\include

ECHO ON
SET DLL_NETCDF=/DDLL_NETCDF
IF %DLLCDF%=="no" SET DLL_NETCDF=
SET TR=
SET TROBJ=
IF %TRIANGLE%=="yes" SET TR=/DTRIANGLE_D
IF %TRIANGLE%=="yes" SET TROBJ=triangle.obj
SET COPT=/DWIN32 /W3 /O2 /nologo %TR% %DLL_NETCDF% /DDLL_PSL /DDLL_GMT %USE_GDAL%
SET DLL=/FD
IF %CHOICE%=="static" SET COPT=/DWIN32 /W3 /O2 /nologo %DLL_NETCDF% %USE_GDAL%
IF %CHOICE%=="static" SET DLL=
set LOPT=/nologo /dll /incremental:no
REM ----------------------------------------------------
ECHO STEP 1: Make PS library
REM ----------------------------------------------------
%CC% %COPT% /c %DLL% /DDLL_EXPORT /DGMT_SHARE_PATH=%GMT_SHARE_PATH% pslib.c
IF %CHOICE%=="dynamic" link %LOPT% /out:psl%BITS%.dll /implib:psl.lib pslib.obj
IF %CHOICE%=="static" lib /out:psl.lib pslib.obj
REM ----------------------------------------------------
ECHO STEP 2: Make GMT library
REM ----------------------------------------------------
%CC% %COPT% /c %DLL% /DDLL_EXPORT /DMIRONE /DGMT_SHARE_PATH=%GMT_SHARE_PATH% gmt_bcr.c gmt_cdf.c gmt_nc.c gmt_customio.c gmt_grdio.c gmt_init.c
%CC% %COPT% /c %DLL% /DDLL_EXPORT /DGMT_SHARE_PATH=%GMT_SHARE_PATH% gmt_io.c gmt_map.c gmt_plot.c gmt_proj.c gmt_shore.c
%CC% %COPT% /c %DLL% /DDLL_EXPORT /DGMT_SHARE_PATH=%GMT_SHARE_PATH% gmt_stat.c gmt_calclock.c gmt_support.c gmt_vector.c
IF %TRIANGLE%=="yes" %CC% %COPT% /c /DNO_TIMER /DTRILIBRARY /DREDUCED /DCDT_ONLY triangle.c
IF %CHOICE%=="dynamic" link %LOPT% /out:gmt%BITS%.dll /implib:gmt.lib gmt_*.obj %TROBJ% psl.lib libnetcdf.lib gdal_i.lib setargv.obj
IF %CHOICE%=="static" lib /out:gmt.lib gmt_*.obj %TROBJ%
REM ----------------------------------------------------
ECHO STEP 3: Make GMT programs 
REM ----------------------------------------------------
set GMTLIB=gmt.lib psl.lib libnetcdf.lib gdal_i.lib setargv.obj
%CC% %COPT% blockmean.c %GMTLIB%
%CC% %COPT% blockmedian.c %GMTLIB%
%CC% %COPT% blockmode.c %GMTLIB%
%CC% %COPT% filter1d.c %GMTLIB%
%CC% %COPT% fitcircle.c %GMTLIB%
%CC% %COPT% gmt2rgb.c %GMTLIB%
%CC% %COPT% gmtconvert.c %GMTLIB%
%CC% %COPT% gmtdefaults.c %GMTLIB%
%CC% %COPT% gmtmath.c %GMTLIB%
%CC% %COPT% gmtselect.c %GMTLIB%
%CC% %COPT% gmtset.c %GMTLIB%
%CC% %COPT% grdfilter.c %GMTLIB%
%CC% %COPT% grd2cpt.c %GMTLIB%
%CC% %COPT% grd2xyz.c %GMTLIB%
%CC% %COPT% grdblend.c %GMTLIB%
%CC% %COPT% grdclip.c %GMTLIB%
%CC% %COPT% grdcontour.c %GMTLIB%
%CC% %COPT% grdcut.c %GMTLIB%
%CC% %COPT% grdedit.c %GMTLIB%
%CC% %COPT% grdfft.c %GMTLIB%
%CC% %COPT% grdgradient.c %GMTLIB%
%CC% %COPT% grdhisteq.c %GMTLIB%
%CC% %COPT% grdimage.c %GMTLIB%
%CC% %COPT% grdinfo.c %GMTLIB%
%CC% %COPT% grdlandmask.c %GMTLIB%
%CC% %COPT% grdmask.c %GMTLIB%
%CC% %COPT% grdmath.c %GMTLIB%
%CC% %COPT% grdpaste.c %GMTLIB%
%CC% %COPT% grdproject.c %GMTLIB%
%CC% %COPT% grdreformat.c %GMTLIB%
%CC% %COPT% grdsample.c %GMTLIB%
%CC% %COPT% grdtrend.c %GMTLIB%
%CC% %COPT% grdtrack.c %GMTLIB%
%CC% %COPT% grdvector.c %GMTLIB%
%CC% %COPT% grdview.c %GMTLIB%
%CC% %COPT% grdvolume.c %GMTLIB%
%CC% %COPT% greenspline.c %GMTLIB%
%CC% %COPT% makecpt.c %GMTLIB%
%CC% %COPT% mapproject.c %GMTLIB%
%CC% %COPT% minmax.c %GMTLIB%
%CC% %COPT% nearneighbor.c %GMTLIB%
%CC% %COPT% project.c %GMTLIB%
%CC% %COPT% ps2raster.c %GMTLIB%
%CC% %COPT% psbasemap.c %GMTLIB%
%CC% %COPT% psclip.c %GMTLIB%
%CC% %COPT% pscoast.c %GMTLIB%
%CC% %COPT% pscontour.c %GMTLIB%
%CC% %COPT% pshistogram.c %GMTLIB%
%CC% %COPT% pslegend.c %GMTLIB%
%CC% %COPT% psimage.c %GMTLIB%
%CC% %COPT% psmask.c %GMTLIB%
%CC% %COPT% psrose.c %GMTLIB%
%CC% %COPT% psscale.c %GMTLIB%
%CC% %COPT% pstext.c %GMTLIB%
%CC% %COPT% pswiggle.c %GMTLIB%
%CC% %COPT% psxy.c %GMTLIB%
%CC% %COPT% psxyz.c %GMTLIB%
%CC% %COPT% sample1d.c %GMTLIB%
%CC% %COPT% spectrum1d.c %GMTLIB%
%CC% %COPT% splitxyz.c %GMTLIB%
%CC% %COPT% surface.c %GMTLIB%
%CC% %COPT% trend1d.c %GMTLIB%
%CC% %COPT% trend2d.c %GMTLIB%
%CC% %COPT% triangulate.c %GMTLIB%
%CC% %COPT% xyz2grd.c %GMTLIB%
REM ----------------------------------------------------
ECHO STEP 4: Clean up and install executables and libraries
REM ----------------------------------------------------
DEL *.obj
MOVE *.exe %BINDIR%
MOVE *.lib %LIBDIR%
COPY *.h %INCDIR%
IF %CHOICE%=="dynamic" MOVE *.dll %BINDIR%
IF %CHOICE%=="dynamic" MOVE *.exp %LIBDIR%
