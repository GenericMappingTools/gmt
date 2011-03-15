@ECHO OFF
REM ----------------------------------------------------
REM
REM	$Id: gmtsuppl.bat,v 1.58 2011-03-15 02:06:36 guru Exp $
REM
REM
REM	Copyright (c) 1991-2010 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
REM	See LICENSE.TXT file for copying and redistribution conditions.
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
REM the GMT 5 supplemental programs under WIN32|64 using
REM Microsoft Visual or Intel C/C++ tools.
REM
REM	We cannot build one program individualy but we can build one of the following groups:
REM	dbase, gshhs, imgsrc, meca, mgd77, misc, potential, segy, spotter, x2sys
REM	To do it, give the group name as argument to this batch. E.G. gmtsuppl meca
REM
REM Author: Joaquim Luis, 06-OCT-2010
REM ----------------------------------------------------
REM
REM How to build GMT under Windows:
REM
REM Before you begin: Install GMT using instructions in gmtinstall.bat

REM
REM STEP a: Specify your compiler (Tested with MS CL and Intel ICL)
SET CC=CL

REM
REM STEP b: Specify the "Bitage" and if building normal or debug version
REM         Set DEBUG to "yes" or "no" and BITS = 32 or 64 (no quotes)
SET DEBUG="no"
SET BITS=64

REM
REM STEP c: Set the environment needed by GMT.  These are
REM	    NETCDF	Top dir of the netcdf installation tree
REM	    lib_netcdf	Name of the netCDF .lib library
REM
SET NETCDF=C:\progs_cygw\netcdf-3.6.3
SET INCLUDE=%INCLUDE%;%NETCDF%\INCLUDE
SET LIB=%LIB%;%NETCDF%\LIB
SET lib_netcdf=libnetcdf_w%BITS%.lib

REM
REM STEP d: Change BINDIR and LIBDIR if necessary (Must be the same as in gmtinstall.bat)
REM
IF %BITS%==64 (
SET BINDIR=..\..\WIN64\bin
SET LIBDIR=..\..\WIN64\lib
SET INCDIR=..\..\WIN64\include\gmt
) ELSE (
SET BINDIR=..\..\WIN32\bin
SET LIBDIR=..\..\WIN32\lib
SET INCDIR=..\..\WIN32\include\gmt
)

REM
REM STEP e: Specify if building in GMT4 compatibility mode
REM STEP    Set to "yes" to build in compatibility mode
SET COMPAT_MODE="yes"
REM
SET COMPAT=
IF  %COMPAT_MODE%=="yes" SET COMPAT=/DGMT_COMPAT

REM
REM STEP f: Dirty trick to allow building a gmt.dll with all supplements included
REM         If set to "y", the .obj files will be copied into src, so if you run
REM         gmtinstall.bat after this all .obj will go into gmt.dll & gmt.lib
REM
SET MOVE_OBJ=="y"

REM ----------------------------------------------------
REM STOP HERE - THE REST IS AUTOMATIC
REM ----------------------------------------------------

SET LDEBUG=
IF  %DEBUG%=="yes" SET LDEBUG=/debug
SET COPTIM=/Ox /DNDEBUG
IF  %DEBUG%=="yes" SET COPTIM=/Z7

SET COMPFLAGS=/W4 /D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE /D_SCL_SECURE_NO_DEPRECATE /D_SECURE_SCL=0 /nologo
SET COPT=/I.. /DWIN32 %OPTIM% /DDLL_NETCDF /DDLL_PSL /DDLL_GMT %COMPFLAGS% %COMPAT%
SET LOPT=/nologo /dll /incremental:no %LDEBUG% 
set GMTLIB=%LIBDIR%\gmt.lib %LIBDIR%\psl.lib %lib_netcdf% setargv.obj

IF "%1" == ""  GOTO todos
IF %1==dbase   GOTO dbase
IF %1==gshhs   GOTO gshhs
IF %1==imgsrc  GOTO imgsrc
IF %1==meca    GOTO meca
IF %1==mgd77   GOTO mgd77
IF %1==misc    GOTO misc
IF %1==potential GOTO potential
IF %1==segy    GOTO segy
IF %1==sph     GOTO sph
IF %1==spotter GOTO spotter
IF %1==x2sys   GOTO x2sys

:todos
:dbase
REM ----------------------------------------------------
ECHO ----------------------------- STEP 1: Make dbase
REM ----------------------------------------------------
cd dbase
%CC% %COPT% /c /DDLL_EXPORT /DGMT_SHARE_PATH=%GMT_SHARE_PATH% *_func.c gmt_dbase.c
link %LOPT% /out:gmt_dbase.dll /implib:gmt_dbase.lib *_func.obj gmt_dbase.obj %GMTLIB%

%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_grdraster /Fegrdraster ../gmtprogram.c gmt_dbase.lib %GMTLIB%

del *.exp *.idb *.ilk gmtprogram.obj
IF %MOVE_OBJ%=="y" (move *.obj ..\)  ELSE (del *.obj)
move *.exe %BINDIR%
move gmt_dbase.dll %BINDIR%
move gmt_dbase.lib %LIBDIR%
COPY *.h %INCDIR%
IF %DEBUG%=="yes" MOVE *.pdb %BINDIR% 
cd ..
IF "%1"=="dbase" GOTO END
REM ----------------------------------------------------

:gshhs
REM ----------------------------------------------------
ECHO ----------------------------- STEP 2: Make gshhs
REM ----------------------------------------------------
cd gshhs
%CC% %COPT% /c /DDLL_EXPORT /DGMT_SHARE_PATH=%GMT_SHARE_PATH% *_func.c
link %LOPT% /out:gmt_gshhs.dll /implib:gmt_gshhs.lib *_func.obj %GMTLIB%

%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_gshhs  /Fegshhs ../gmtprogram.c gmt_gshhs.lib %GMTLIB%

rem %CC% %COPT% gshhs_dp.c %GMTLIB%
rem %CC% %COPT% gshhstograss.c %GMTLIB%

del *.exp *.idb *.ilk gmtprogram.obj
IF %MOVE_OBJ%=="y" (move *.obj ..\)  ELSE (del *.obj)
move *.exe %BINDIR%
move gmt_gshhs.dll %BINDIR%
move gmt_gshhs.lib %LIBDIR%
COPY *.h %INCDIR%
IF %DEBUG%=="yes" MOVE *.pdb %BINDIR%
cd ..
IF "%1"=="gshhs" GOTO END
REM ----------------------------------------------------

:imgsrc
REM ----------------------------------------------------
ECHO ----------------------------- STEP 3: Make imgsrc
REM ----------------------------------------------------
cd imgsrc
%CC% %COPT% /c /DDLL_EXPORT /DGMT_SHARE_PATH=%GMT_SHARE_PATH% *_func.c gmt_imgsubs.c gmt_img.c
link %LOPT% /out:gmt_img.dll /implib:gmt_img.lib *_func.obj gmt_imgsubs.obj gmt_img.obj %GMTLIB%

%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_img2grd  /Feimg2grd ../gmtprogram.c gmt_img.lib %GMTLIB%

del *.exp *.idb *.ilk gmtprogram.obj
IF %MOVE_OBJ%=="y" (move *.obj ..\)  ELSE (del *.obj)
move *.exe %BINDIR%
move gmt_img.dll %BINDIR%
move gmt_img.lib %LIBDIR%
COPY *.h %INCDIR%
IF %DEBUG%=="yes" MOVE *.pdb %BINDIR% 
cd ..
IF "%1"=="imgsrc" GOTO END
REM ----------------------------------------------------

:meca
REM ----------------------------------------------------
ECHO ----------------------------- STEP 4: Make meca
REM ----------------------------------------------------
cd meca
%CC% %COPT% /c /DDLL_EXPORT /DGMT_SHARE_PATH=%GMT_SHARE_PATH% *_func.c nrutil.c distaz.c submeca.c utilmeca.c gmt_meca.c
link %LOPT% /out:gmt_meca.dll /implib:gmt_meca.lib *_func.obj distaz.obj nrutil.obj submeca.obj utilmeca.obj gmt_meca.obj %GMTLIB%

%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_pscoupe /Fepscoupe ../gmtprogram.c gmt_meca.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_pspolar /Fepspolar ../gmtprogram.c gmt_meca.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_psmeca /Fepsmeca ../gmtprogram.c gmt_meca.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_psvelo /Fepsvelo ../gmtprogram.c gmt_meca.lib %GMTLIB%

del *.exp *.idb *.ilk gmtprogram.obj
IF %MOVE_OBJ%=="y" (move *.obj ..\)  ELSE (del *.obj)
move *.exe %BINDIR%
move gmt_meca.dll %BINDIR%
move gmt_meca.lib %LIBDIR%
COPY *.h %INCDIR%
IF %DEBUG%=="yes" MOVE *.pdb %BINDIR% 
cd ..
IF "%1"=="meca" GOTO END
REM ----------------------------------------------------

ECHO ----------------------------- STEP 5: Make mex
REM ----------------------------------------------------
echo See the compile_mex.bat in mex sub-dir on how to make mex files
REM ----------------------------------------------------

:mgd77
REM ----------------------------------------------------
ECHO ----------------------------- STEP 6: Make mgd77
REM ----------------------------------------------------
cd mgd77
%CC% %COPT% /c /DDLL_EXPORT /DGMT_SHARE_PATH=%GMT_SHARE_PATH% *_func.c mgd77.c gmt_mgd77.c
link %LOPT% /out:gmt_mgd77.dll /implib:gmt_mgd77.lib *_func.obj mgd77.obj gmt_mgd77.obj %GMTLIB%

%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_mgd77convert /Femgd77convert ../gmtprogram.c gmt_mgd77.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_mgd77info    /Femgd77info    ../gmtprogram.c gmt_mgd77.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_mgd77list    /Femgd77list    ../gmtprogram.c gmt_mgd77.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_mgd77manage  /Femgd77manage  ../gmtprogram.c gmt_mgd77.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_mgd77path    /Femgd77path    ../gmtprogram.c gmt_mgd77.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_mgd77sniffer /Femgd77sniffer ../gmtprogram.c gmt_mgd77.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_mgd77track   /Femgd77track   ../gmtprogram.c gmt_mgd77.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_mgd77magref  /Femgd77magref  ../gmtprogram.c gmt_mgd77.lib %GMTLIB%

del *.exp *.idb *.ilk gmtprogram.obj
IF %MOVE_OBJ%=="y" (move *.obj ..\)  ELSE (del *.obj)
move *.exe %BINDIR%
move gmt_mgd77.dll %BINDIR%
move gmt_mgd77.lib %LIBDIR%
COPY *.h %INCDIR%
IF %DEBUG%=="yes" MOVE *.pdb %BINDIR% 
cd ..
IF "%1"=="mgd77" GOTO END
REM ----------------------------------------------------

:misc
REM ----------------------------------------------------
ECHO ----------------------------- STEP 7: Make misc
REM ----------------------------------------------------
cd misc
%CC% %COPT% dimfilter.c %GMTLIB%

del *.exp *.idb *.ilk *.obj
move *.exe %BINDIR%
IF %DEBUG%=="yes" MOVE *.pdb %BINDIR% 
cd ..
IF "%1"=="misc" GOTO END
REM ----------------------------------------------------

:potential
REM ----------------------------------------------------
ECHO ----------------------------- STEP 8: Make potential
REM ----------------------------------------------------
cd potential
%CC% %COPT% /c /DDLL_EXPORT /DGMT_SHARE_PATH=%GMT_SHARE_PATH% *_func.c gmt_potential.c
link %LOPT% /out:gmt_potential.dll /implib:gmt_potential.lib *_func.obj gmt_potential.obj %GMTLIB%

%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_redpol /Feredpol ../gmtprogram.c gmt_potential.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_xyzokb /Fexyzokb ../gmtprogram.c gmt_potential.lib %GMTLIB%

del *.exp *.idb *.ilk gmtprogram.obj
IF %MOVE_OBJ%=="y" (move *.obj ..\)  ELSE (del *.obj)
move *.exe %BINDIR%
move gmt_potential.dll %BINDIR%
move gmt_potential.lib %LIBDIR%
COPY *.h %INCDIR%
IF %DEBUG%=="yes" MOVE *.pdb %BINDIR% 
cd ..
IF "%1"=="potential" GOTO END
REM ----------------------------------------------------

:segy
REM ----------------------------------------------------
ECHO ----------------------------- STEP 9: Make segy
REM ----------------------------------------------------
cd segyprogs
%CC% %COPT% /c /DDLL_EXPORT /DGMT_SHARE_PATH=%GMT_SHARE_PATH% *_func.c segy_io.c gmt_segy.c
link %LOPT% /out:gmt_segy.dll /implib:gmt_segy.lib *_func.obj segy_io.obj gmt_segy.obj %GMTLIB%

%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_pssegy /Fepssegy ../gmtprogram.c gmt_segy.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMTPSL /DFUNC=GMT_pssegyz /Fepssegyz ../gmtprogram.c gmt_segy.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_segy2grd /Fesegy2grd ../gmtprogram.c gmt_segy.lib %GMTLIB%

del *.exp *.idb *.ilk gmtprogram.obj
IF %MOVE_OBJ%=="y" (move *.obj ..\)  ELSE (del *.obj)
move *.exe %BINDIR%
move gmt_segy.dll %BINDIR%
move gmt_segy.lib %LIBDIR%
COPY *.h %INCDIR%
IF %DEBUG%=="yes" MOVE *.pdb %BINDIR% 
cd ..
IF "%1"=="segy" GOTO END
REM ----------------------------------------------------

:sph
REM ----------------------------------------------------
ECHO ----------------------------- STEP 10: Make sph
REM ----------------------------------------------------
cd sph
%CC% %COPT% /c /DDLL_EXPORT /DGMT_SHARE_PATH=%GMT_SHARE_PATH% *_func.c sph.c gmt_sph.c
link %LOPT% /out:gmt_sph.dll /implib:gmt_sph.lib *_func.obj sph.obj gmt_sph.obj %GMTLIB%

%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_sphtriangulate /Fesphtriangulate ../gmtprogram.c gmt_sph.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_sphdistance    /Fesphdistance    ../gmtprogram.c gmt_sph.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_sphinterpolate /Fesphinterpolate ../gmtprogram.c gmt_sph.lib %GMTLIB%

del *.exp *.idb *.ilk gmtprogram.obj
IF %MOVE_OBJ%=="y" (move *.obj ..\)  ELSE (del *.obj)
move *.exe %BINDIR%
move gmt_sph.dll %BINDIR%
move gmt_sph.lib %LIBDIR%
COPY *.h %INCDIR%
IF %DEBUG%=="yes" MOVE *.pdb %BINDIR% 
cd ..
IF "%1"=="sph" GOTO END
REM ----------------------------------------------------

:spotter
REM ----------------------------------------------------
ECHO ----------------------------- STEP 11: Make spotter
REM ----------------------------------------------------
cd spotter
%CC% %COPT% /c /DDLL_EXPORT /DGMT_SHARE_PATH=%GMT_SHARE_PATH% *_func.c libspotter.c
link %LOPT% /out:gmt_spotter.dll /implib:gmt_spotter.lib *_func.obj libspotter.obj %GMTLIB%

%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_backtracker  /Febacktracker  ../gmtprogram.c gmt_spotter.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_grdrotater   /Fegrdrotater   ../gmtprogram.c gmt_spotter.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_grdspotter   /Fegrdspotter   ../gmtprogram.c gmt_spotter.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_hotspotter   /Fehotspotter   ../gmtprogram.c gmt_spotter.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_originator   /Feoriginator   ../gmtprogram.c gmt_spotter.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_rotconverter /Ferotconverter ../gmtprogram.c gmt_spotter.lib %GMTLIB%

del *.exp *.idb *.ilk gmtprogram.obj
IF %MOVE_OBJ%=="y" (move *.obj ..\)  ELSE (del *.obj)
move *.exe %BINDIR%
move gmt_spotter.dll %BINDIR%
move gmt_spotter.lib %LIBDIR%
COPY *.h %INCDIR%
IF %DEBUG%=="yes" MOVE *.pdb %BINDIR% 
cd ..
IF "%1"=="spotter" GOTO END
REM ----------------------------------------------------

:x2sys
REM ----------------------------------------------------
ECHO ----------------------------- STEP 12: Make x2sys
REM ----------------------------------------------------
cd x2sys
%CC% %COPT% /c /DDLL_EXPORT /DGMT_SHARE_PATH=%GMT_SHARE_PATH% /I..\mgd77 *_func.c x2sys.c gmt_x2sys.c
link %LOPT% /out:gmt_x2sys.dll /implib:gmt_x2sys.lib *_func.obj x2sys.obj gmt_x2sys %LIBDIR%\gmt_mgd77.lib %GMTLIB%

%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_x2sys_binlist  /Fex2sys_binlist  /I..\mgd77 ../gmtprogram.c gmt_x2sys.lib %LIBDIR%\gmt_mgd77.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_x2sys_cross    /Fex2sys_cross    /I..\mgd77 ../gmtprogram.c gmt_x2sys.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_x2sys_datalist /Fex2sys_datalist /I..\mgd77 ../gmtprogram.c gmt_x2sys.lib %LIBDIR%\gmt_mgd77.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_x2sys_get      /Fex2sys_get      /I..\mgd77 ../gmtprogram.c gmt_x2sys.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_x2sys_init     /Fex2sys_init     /I..\mgd77 ../gmtprogram.c gmt_x2sys.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_x2sys_put      /Fex2sys_put      /I..\mgd77 ../gmtprogram.c gmt_x2sys.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_x2sys_list     /Fex2sys_list     /I..\mgd77 ../gmtprogram.c gmt_x2sys.lib %LIBDIR%\gmt_mgd77.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_x2sys_report   /Fex2sys_report   /I..\mgd77 ../gmtprogram.c gmt_x2sys.lib %LIBDIR%\gmt_mgd77.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_x2sys_solve    /Fex2sys_solve    /I..\mgd77 ../gmtprogram.c gmt_x2sys.lib %GMTLIB%
%CC% %COPT% /DFUNC_MODE=GMTAPI_GMT /DFUNC=GMT_x2sys_merge    /Fex2sys_merge ../gmtprogram.c gmt_x2sys.lib %GMTLIB%

del *.exp *.idb *.ilk gmtprogram.obj
IF %MOVE_OBJ%=="y" (move *.obj ..\)  ELSE (del *.obj)
move *.exe %BINDIR%
move gmt_x2sys.dll %BINDIR%
move gmt_x2sys.lib %LIBDIR%
COPY *.h %INCDIR%
IF %DEBUG%=="yes" MOVE *.pdb %BINDIR% 
cd ..

:END
