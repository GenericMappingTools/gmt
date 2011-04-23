@ECHO OFF
REM ----------------------------------------------------
REM
REM	$Id: gmtinstall.bat,v 1.58 2011-04-23 02:14:12 guru Exp $
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
REM	Contact info: gmt.soest.hawaii.edu
REM --------------------------------------------------------------------
REM This extremely lame DOS batch file will compile
REM the GMT 5 suite of programs under WIN32|64 using
REM Microsoft Visual or Intel C/C++ tools.  It will build GMT
REM using DLL libraries. 
REM
REM Author: Joaquim Luis & Paul Wessel, 06-OCT-2010
REM ----------------------------------------------------
REM
REM How to build GMT executables under Windows:
REM Actually, before using this batch files successfully, several files need to be generated
REM from the GMT tree. On *nix this is done by the configure script, but on Windows we
REM currently have no way to do the same job. So in practice, to build the Windows version
REM it is necessary to build it first under Cygwin.

REM
REM STEP a: Specify your compiler (Tested with MS CL and Intel ICL)
SET CC=CL

REM
REM STEP b: Specify the "Bitage" and if building normal or debug version
REM         Set DEBUG to "yes" or "no" and BITS = 32 or 64 (no quotes)
SET DEBUG="yes"
SET BITS=64

REM
REM STEP c: Install netcdf 3.6.3 as provided by the GMT site
REM         and adjust below for your own paths.
REM	    Note: If you are using other netcdf lib_netcdf may have a different name
REM	    NETCDF	Top dir of the netcdf installation tree
REM	    lib_netcdf	Name of the netCDF .lib library
REM
REM
SET NETCDF=C:\progs_cygw\netcdf-3.6.3
SET INCLUDE=%INCLUDE%;%NETCDF%\INCLUDE
SET LIB=%LIB%;%NETCDF%\LIB
SET lib_netcdf=libnetcdf_w%BITS%.lib

REM STEP  : Set the environment needed by GMT.  These are
REM	    GMTHOME	Top dir of the GMT installation tree, e.g., C:\GMT
REM	    GMT_SHAREDIR	Place to read GMT run-time files.  If not
REM				set we use %GMTHOME%\share
REM	    PATH	Add the path to the netCDF DLL library netcdf.dll
REM	    PATH	Add the path to the GMT executables (%GMTHOME%\bin)

REM STEP d: Check/edit definitions of BINDIR, LIBDIR, and GMT_SHARE_PATH.
REM	    Make sure BINDIR below points to a valid directory
REM	    where you want executables to be installed. Either
REM	    edit BINDIR or create the ..\bin directory.
REM	    Same goes for LIBDIR where GMT libraries will be kept.
REM	    GMT_SHARE_PATH is where GMT expects to find the shared data.
REM	    It is ONLY used if the user does not set %GMT_SHAREDIR%.
REM
IF %BITS%==64 (
SET BINDIR=..\WIN64\bin
SET LIBDIR=..\WIN64\lib
SET INCDIR=..\WIN64\include\gmt
) ELSE (
SET BINDIR=..\WIN32\bin
SET LIBDIR=..\WIN32\lib
SET INCDIR=..\WIN32\include\gmt
)
SET GMT_SHARE_PATH="\"C:\\programs\\GMT\\share\""

REM STEP e: To optionally link against the GDAL library you must set
REM	    GDAL to "yes" or pass gdal as 2nd argument to this script
REM	    Set the right path in GDAL_INC & GDAL_LIB to reflect
REM	    your FWTools installation.
SET GDAL="yes"
IF  "%2%" == "gdal" set GDAL="yes"
SET GDAL_INC=
SET GDAL_LIB=
SET USE_GDAL=
REM IF %GDAL%=="yes" SET GDAL_INC=/IC:\programs\FWTools2.4.3\include 
REM IF %GDAL%=="yes" SET GDAL_LIB=C:\programs\FWTools2.4.3\lib\gdal_i.lib 
IF  %GDAL%=="yes" SET GDAL_INC="/IC:\programs\GDALtrunk\gdal\compileds\VC10_%BITS%\include" 
IF  %GDAL%=="yes" SET GDAL_LIB=C:\programs\GDALtrunk\gdal\compileds\VC10_%BITS%\lib\gdal_i.lib 
IF  %GDAL%=="yes" SET USE_GDAL=/DUSE_GDAL

REM STEP f:  Set to "yes" to build only a gmt.dll version specially crafted for MATLAB
REM	     or pass ML as 1st argument to this script:
REM	     Adjust also MATLIB & MATINC variables to your own path
REM
SET forMATLAB="no"
IF  "%1%" == "ML" set forMATLAB="yes"
REM
IF  %forMATLAB%=="yes" (
SET MATLIB=/LIBPATH:C:\PROGRAMS\MATLAB\R2010A\extern\lib\win%BITS%\microsoft libmex.lib 
SET MATINC=-IC:\PROGRAMS\MATLAB\R2010A\extern\include
SET _MX_COMPAT=-DMX_COMPAT_32
SET TO_MATLAB=/DGMT_MATLAB
) ELSE (
SET MATLIB=
SET MATINC=
SET _MX_COMPAT=
SET TO_MATLAB=
)

REM STEP g: Specify if building in GMT4 compatibility mode
REM STEP    Set to "yes" to build in compatibility mode
SET COMPAT_MODE="yes"
REM
SET COMPAT=
IF  %COMPAT_MODE%=="yes" SET COMPAT=/DGMT_COMPAT

REM ----------------------------------------------------
REM STOP HERE - THE REST IS AUTOMATIC
REM ----------------------------------------------------

SET LDEBUG=
IF  %DEBUG%=="yes" SET LDEBUG=/debug
SET COPTIM=/Ox /DNDEBUG
IF  %DEBUG%=="yes" SET OPTIM=/Z7

SET DLL_NETCDF=/DDLL_NETCDF
SET TR=/DTRIANGLE_D

SET COMPFLAGS=/W3 /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE /D_SCL_SECURE_NO_DEPRECATE /D_SECURE_SCL=0 /nologo
SET COPT=/DWIN32 %OPTIM% %TR% %DLL_NETCDF% /DDLL_PSL /DDLL_GMT %USE_GDAL% %GDAL_INC% %TO_MATLAB% %COMPFLAGS% %COMPAT%

set LOPT=/nologo /dll /incremental:no %LDEBUG%
REM ----------------------------------------------------
ECHO STEP 1: Make PS library
REM ----------------------------------------------------
%CC% %COPT% /c /DDLL_EXPORT /DGMT_SHARE_PATH=%GMT_SHARE_PATH% pslib.c
link %LOPT% /out:psl.dll /implib:psl.lib pslib.obj
DEL pslib.obj
REM ----------------------------------------------------
ECHO STEP 2: Make GMT library
REM ----------------------------------------------------
%CC% %COPT% /c /DDLL_EXPORT /DGMT_SHARE_PATH=%GMT_SHARE_PATH% gmt_bcr.c gmt_cdf.c gmt_nc.c gmt_customio.c gmt_grdio.c
%CC% %COPT% /c /DDLL_EXPORT /DGMT_SHARE_PATH=%GMT_SHARE_PATH% %MATINC% gmt_init.c gmt_io.c gmt_map.c gmt_plot.c gmt_proj.c gmt_shore.c
%CC% %COPT% /c /DDLL_EXPORT /DGMT_SHARE_PATH=%GMT_SHARE_PATH% gmt_fft.c gmt_stat.c gmt_calclock.c gmt_support.c gmt_vector.c
%CC% %COPT% /c /DDLL_EXPORT /DGMT_SHARE_PATH=%GMT_SHARE_PATH% gmtapi_parse.c gmtapi_util.c
%CC% %COPT% /c /DDLL_EXPORT /DGMT_SHARE_PATH=%GMT_SHARE_PATH% *_func.c gmt_modules.c

%CC% %COPT% /c /DNO_TIMER /DTRILIBRARY /DREDUCED /DCDT_ONLY triangle.c

REM Note: the following form will build a gmt.dll with all supplements
REM if MOVE_OBJ was set to "y" in a previous run of gmtsuppl.bat
link %LOPT% /out:gmt.dll /implib:gmt.lib psl.lib %lib_netcdf% %GDAL_LIB% %MATLIB% *.obj

IF %forMATLAB%=="yes" DEL *.obj *.lib *.exp psl.dll
IF %forMATLAB%=="yes" GOTO fim
REM ----------------------------------------------------
ECHO STEP 3: Make GMT programs 
REM ----------------------------------------------------
set GMTLIB=gmt.lib %lib_netcdf% setargv.obj

%CC% %COPT% testio.c %GMTLIB%

%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_blockmean    /Feblockmean    gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_blockmedian  /Feblockmedian  gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_blockmode    /Feblockmode    gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_filter1d     /Fefilter1d     gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_fitcircle    /Fefitcircle    gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_gmt2kml      /Fegmt2kml      gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_gmtconvert   /Fegmtconvert   gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_gmtdefaults  /Fegmtdefaults  gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_gmtdp        /Fegmtdp        gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_gmtmath      /Fegmtmath      gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_gmtselect    /Fegmtselect    gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_gmtset       /Fegmtset       gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_gmtspatial   /Fegmtspatial   gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_gmtvector    /Fegmtvector    gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_gmtstitch    /Fegmtstitch    gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_grdfilter    /Fegrdfilter    gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_grd2cpt      /Fegrd2cpt      gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_grd2rgb      /Fegrd2rgb      gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_grd2xyz      /Fegrd2xyz      gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_grdblend     /Fegrdblend     gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_grdclip      /Fegrdclip      gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_grdcontour   /Fegrdcontour   gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_grdcut       /Fegrdcut       gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_grdedit      /Fegrdedit      gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_grdfft       /Fegrdfft       gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_grdgradient  /Fegrdgradient  gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_grdhisteq    /Fegrdhisteq    gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_grdimage     /Fegrdimage     gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_grdinfo      /Fegrdinfo      gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_grdlandmask  /Fegrdlandmask  gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_grdmask      /Fegrdmask      gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_grdmath      /Fegrdmath      gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_grdpaste     /Fegrdpaste     gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_grdproject   /Fegrdproject   gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_grdreformat  /Fegrdreformat  gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_grdsample    /Fegrdsample    gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_grdtrend     /Fegrdtrend     gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_grdtrack     /Fegrdtrack     gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_grdvector    /Fegrdvector    gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_grdview      /Fegrdview      gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_grdvolume    /Fegrdvolume    gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_greenspline  /Fegreenspline  gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_kml2gmt      /Fekml2gmt      gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_makecpt      /Femakecpt      gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_mapproject   /Femapproject   gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_minmax       /Feminmax       gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_nearneighbor /Fenearneighbor gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_project      /Feproject      gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_ps2raster    /Feps2raster    gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_psbasemap    /Fepsbasemap    gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_psclip       /Fepsclip       gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_pscoast      /Fepscoast      gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_pscontour    /Fepscontour    gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_pshistogram  /Fepshistogram  gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_pslegend     /Fepslegend     gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_psimage      /Fepsimage      gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_psmask       /Fepsmask       gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_psrose       /Fepsrose       gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_psscale      /Fepsscale      gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_pstext       /Fepstext       gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_pswiggle     /Fepswiggle     gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_psxy         /Fepsxy         gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_psxyz        /Fepsxyz        gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_sample1d     /Fesample1d     gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_spectrum1d   /Fespectrum1d   gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_splitxyz     /Fesplitxyz     gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_surface      /Fesurface      gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_trend1d      /Fetrend1d      gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_trend2d      /Fetrend2d      gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_triangulate  /Fetriangulate  gmtprogram.c %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT    /DFUNC=GMT_xyz2grd      /Fexyz2grd      gmtprogram.c %GMTLIB%
REM ----------------------------------------------------
ECHO STEP 4: Clean up and install executables and libraries
REM ----------------------------------------------------
DEL *.obj
MOVE *.exe %BINDIR%
MOVE *.lib %LIBDIR%
COPY *.h %INCDIR%
MOVE *.dll %BINDIR%
MOVE *.exp %LIBDIR%
IF %DEBUG%=="yes" MOVE *.pdb %BINDIR% 
DEL *.ilk *.idb
:fim
