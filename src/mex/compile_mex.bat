@echo off
REM ----------------------------------------------------
REM
REM	$Id$
REM
REM
REM	Copyright (c) 1991-2010 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
REM	See LICENSE.TXT file for copying and redistribution conditions.
REM
REM	This program is free software; you can redistribute it and/or modify
REM	it under the terms of the GNU Lesser General Public License as published by
REM	the Free Software Foundation; version 3 or any later version.
REM
REM	This program is distributed in the hope that it will be useful,
REM	but WITHOUT ANY WARRANTY; without even the implied warranty of
REM	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
REM	GNU Lesser General Public License for more details.
REM
REM	Contact info: gmt.soest.hawaii.edu
REM --------------------------------------------------------------------
REM --------------------------------------------------------------------------------------
REM
REM This is a compile batch that builds all GMT MEXs. Contrary to the 'mex' command it doesn't
REM need you to setup a compiler within MATLAB, which means you can use any of the MS or
REM Intel compilers (a luxury that you don't have with the 'mex' command).
REM
REM If a WIN64 version is targeted than both GMT & netCDF Libs must have been build in 64-bits as well.
REM
REM
REM Usage: open the command window set up by the compiler of interest (were all vars are already set)
REM	   and run this batch from there.
REM 	   NOTE: you must make some edits to the setup below.
REM
REM --------------------------------------------------------------------------------------

REM ------------- Set the compiler (set to 'icl' to use the Intel compiler) --------------
SET CC=cl
REM --------------------------------------------------------------------------------------

REM If set to "yes", linkage is done againsts ML6.5 Libs
SET R13="no"

REM Set it to "yes" or "no" to build under 64-bits or 32-bits respectively.
SET WIN64="yes"

REM The MSVC version. I use this var to select libraries also compiled with this compiler
SET MSVC_VER="1600"

REM
REM Set to "yes" if you want to build a debug version
SET DEBUG="yes"
REM
SET LDEBUG=
IF %DEBUG%=="yes" SET LDEBUG=/debug

REM ------------------ Sets the MATLAB libs and include path ----------------------------
IF %R13%=="yes" (

SET MATLIB=C:\SVN\MAT65_pracompa\extern\lib\win32\microsoft
SET MATINC=C:\SVN\MAT65_pracompa\extern\include
SET _MX_COMPAT=
SET MEX_EXT="dll"

) ELSE (

IF %WIN64%=="yes" (
SET MATLIB=C:\PROGRAMS\MATLAB\R2010B\extern\lib\win64\microsoft
SET MATINC=C:\PROGRAMS\MATLAB\R2010B\extern\include
SET _MX_COMPAT=-DMX_COMPAT_32
SET MEX_EXT="mexw64"

) ELSE (

SET MATLIB=C:\PROGRAMS\MATLAB32\R2010B\extern\lib\win32\microsoft
SET MATINC=C:\PROGRAMS\MATLAB32\R2010B\extern\include
SET _MX_COMPAT=-DMX_COMPAT_32
SET MEX_EXT="mexw32"
) )

REM -------------- Set GMT & NetCDF lib and include ----------------------------
IF %WIN64%=="yes" (

SET NETCDF_LIB=C:\progs_cygw\netcdf-3.6.3\compileds\VC10_64\lib\libnetcdf.lib
SET GMT_LIB=c:\progs_cygw\GMTdev\GMT5\WIN64\lib\gmt.lib
SET GMT_MGG_LIB=c:\progs_cygw\GMTdev\GMT_win64\lib\gmt_mgg.lib

) ELSE (

IF %MSVC_VER%=="1600" (
SET NETCDF_LIB=C:\progs_cygw\netcdf-3.6.3\compileds\VC10_32\lib\libnetcdf.lib
SET GMT_LIB=c:\progs_cygw\GMTdev\GMT5\WIN32\lib\gmt.lib
SET GMT_MGG_LIB=c:\progs_cygw\GMTdev\GMT_win\lib\gmt_mgg.lib

) ELSE (

SET NETCDF_LIB=C:\progs_cygw\netcdf-3.6.3\lib\libnetcdf_w32.lib
SET GMT_LIB=c:\progs_cygw\GMTdev\GMT_win\lib\gmt.lib
SET GMT_MGG_LIB=c:\progs_cygw\GMTdev\GMT_win\lib\gmt_mgg.lib
) )

SET NETCDF_INC=C:\progs_cygw\netcdf-3.6.3\include
SET GMT_INC=c:\progs_cygw\GMTdev\GMT5\src
REM ----------------------------------------------------------------------------

REM ____________________________________________________________________________
REM ___________________ STOP EDITING HERE ______________________________________


SET COMPFLAGS=/c /Zp8 /GR /EHs /D_CRT_SECURE_NO_DEPRECATE /D_SCL_SECURE_NO_DEPRECATE /D_SECURE_SCL=0 /DMATLAB_MEX_FILE /nologo /MD
SET OPTIMFLAGS=/Ox /Oy- /DNDEBUG
IF %DEBUG%=="yes" SET OPTIMFLAGS=/Z7

IF %WIN64%=="yes" SET arc=X64
IF %WIN64%=="no" SET arc=X86
SET LINKFLAGS=/dll /export:mexFunction /LIBPATH:%MATLIB% libmx.lib libmex.lib libmat.lib /MACHINE:%arc% kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /incremental:NO %LDEBUG%

REM -------------------------------------------------------------------------------------------------------
%CC% -DWIN32 %COMPFLAGS% -I%MATINC% -I%NETCDF_INC% -I%GMT_INC% %OPTIMFLAGS% %_MX_COMPAT% -DLIBRARY_EXPORTS -DGMT_MATLAB gmt_mex.c
for %%G in (blockmean gmtconvert grdinfo grdread grdtrack grdwrite mapproject nearneighbor psxy pscoast surface) do (

%CC% -DWIN32 %COMPFLAGS% -I%MATINC% -I%NETCDF_INC% -I%GMT_INC% %OPTIMFLAGS% %_MX_COMPAT% -DLIBRARY_EXPORTS -DGMT_MATLAB %%G.c
link  /out:"%%G.%MEX_EXT%" %LINKFLAGS% %NETCDF_LIB% %GMT_LIB% /implib:templib.x gmt_mex.obj %%G.obj
)
REM -------------------------------------------------------------------------------------------------------

REM --------- These follow the same aproach as with the gmtprogram.c template -----------------------------
%CC% -DWIN32 %COMPFLAGS% -I%MATINC% -I%NETCDF_INC% -I%GMT_INC% %OPTIMFLAGS% %_MX_COMPAT% -DLIBRARY_EXPORTS -DGMT_MATLAB /DFUNC=GMT_grdsample_cmd /Fegrdsample grd2grd.c
link  /out:"grdsample.%MEX_EXT%" %LINKFLAGS% %NETCDF_LIB% %GMT_LIB% /implib:templib.x gmt_mex.obj grd2grd.obj

%CC% -DWIN32 %COMPFLAGS% -I%MATINC% -I%NETCDF_INC% -I%GMT_INC% %OPTIMFLAGS% %_MX_COMPAT% -DLIBRARY_EXPORTS -DGMT_MATLAB /DFUNC=GMT_grdfilter_cmd /Fegrdfilter grd2grd.c
link  /out:"grdfilter.%MEX_EXT%" %LINKFLAGS% %NETCDF_LIB% %GMT_LIB% /implib:templib.x gmt_mex.obj grd2grd.obj

%CC% -DWIN32 %COMPFLAGS% -I%MATINC% -I%NETCDF_INC% -I%GMT_INC% %OPTIMFLAGS% %_MX_COMPAT% -DLIBRARY_EXPORTS -DGMT_MATLAB /DFUNC=GMT_grdfft_cmd /Fegrdfft grd2grd.c
link  /out:"grdfft.%MEX_EXT%" %LINKFLAGS% %NETCDF_LIB% %GMT_LIB% /implib:templib.x gmt_mex.obj grd2grd.obj

%CC% -DWIN32 %COMPFLAGS% -I%MATINC% -I%NETCDF_INC% -I%GMT_INC% %OPTIMFLAGS% %_MX_COMPAT% -DLIBRARY_EXPORTS -DGMT_MATLAB /DFUNC=GMT_grdcut_cmd /Fegrdcut grd2grd.c
link  /out:"grdcut.%MEX_EXT%" %LINKFLAGS% %NETCDF_LIB% %GMT_LIB% /implib:templib.x gmt_mex.obj grd2grd.obj

%CC% -DWIN32 %COMPFLAGS% -I%MATINC% -I%NETCDF_INC% -I%GMT_INC% %OPTIMFLAGS% %_MX_COMPAT% -DLIBRARY_EXPORTS -DGMT_MATLAB /DFUNC=GMT_grdclip_cmd /Fegrdclip grd2grd.c
link  /out:"grdclip.%MEX_EXT%" %LINKFLAGS% %NETCDF_LIB% %GMT_LIB% /implib:templib.x gmt_mex.obj grd2grd.obj

%CC% -DWIN32 %COMPFLAGS% -I%MATINC% -I%NETCDF_INC% -I%GMT_INC% %OPTIMFLAGS% %_MX_COMPAT% -DLIBRARY_EXPORTS -DGMT_MATLAB /DFUNC=GMT_grdhisteq_cmd /Fegrdhisteq grd2grd.c
link  /out:"grdhisteq.%MEX_EXT%" %LINKFLAGS% %NETCDF_LIB% %GMT_LIB% /implib:templib.x gmt_mex.obj grd2grd.obj

%CC% -DWIN32 %COMPFLAGS% -I%MATINC% -I%NETCDF_INC% -I%GMT_INC% %OPTIMFLAGS% %_MX_COMPAT% -DLIBRARY_EXPORTS -DGMT_MATLAB /DFUNC=GMT_grdproject_cmd /Fegrdproject grd2grd.c
link  /out:"grdproject.%MEX_EXT%" %LINKFLAGS% %NETCDF_LIB% %GMT_LIB% /implib:templib.x gmt_mex.obj grd2grd.obj
REM -------------------------------------------------------------------------------------------------------


del *.obj *.exp templib.x
