ECHO OFF
REM	$Id: GMT_winbuild.bat,v 1.52 2011-07-08 23:17:25 guru Exp $
REM	Compiles GMT and builds installers under Windows.
REM	See separate GSHHS_winbuild.bat for GSHHS full+high installer
REM	Paul Wessel with help from Joaquim Luis
REM
REM	Assumptions:
REM	1. You have run make tar_all tar_coast
REM	2. You have placed netcdf in C:\GMTdev\netcdf-3.6.3\VC10_32|64
REM	3. You have placed gdal in C:\GMTdev\gdal\VC10_32|64
REM	4. You have placed gawk.exe in C:\GMTdev\GNU
REM	5. You have C:\GMTdev with dirs INFO and INSTALLERS
REM	6. Inno Setup 5 has been installed and the path
REM	   to its command line tool is added to PATH
REM	7. 7zip has been installed and the path
REM	   to its command line tool is added to PATH
REM
REM To build 32-bit installer, run GMT_winbuild drive 32
REM To build 64-bit installer, run GMT_winbuild drive 64 (ASSUMES 32 was run first!)

REM MAKE SURE THESE TWO ARE UPDATED!
SET GVER=5.0.0b
SET GSHHS=2.2.0

IF "%1%" == "home" (
	SET GMTDIR=W:\RESEARCH\CVSPROJECTS\GMTdev\GMT4
) ELSE (
	SET GMTDIR=%1%:\UH\RESEARCH\CVSPROJECTS\GMTdev\GMT4
)
IF "%2%" == "64" (
	SET BITS=64
) ELSE (
	SET BITS=32
)
set NETCDF_DIR=C:\GMTdev\netcdf-3.6.3\VC10_%BITS%
set GDAL_DIR=C:\GMTdev\gdal\VC10_%BITS%
set GNU_DIR=C:\GMTdev\GNU

C:
IF "%BITS%" == "32" (
	echo === 0. Get all GMT%GVER% bzipped tar balls and extract files...

	cd C:\GMTdev
	copy %GMTDIR%\ftp\GMT%GVER%*.tar.bz2 C:\GMTdev\
	copy %GMTDIR%\ftp\GSHHS%GSHHS%_*.tar.bz2 C:\GMTdev\
	7z x GMT*.tar.bz2
	7z x GSHHS*.tar.bz2
	7z x GMT*.tar -aoa
	7z x GSHHS*.tar -oGMT%GVER% -aoa
	del *.tar.bz2
	del *.tar
	rename GMT%GVER% GMT
	copy %GMTDIR%\src\gmt_version.h C:\GMTdev\GMT\src
	copy %GMTDIR%\src\gmt_notposix.h C:\GMTdev\GMT\src
	copy %GMTDIR%\share\conf\gmt.conf.win C:\GMTdev\GMT\share\conf\gmt.conf
	copy %GMTDIR%\share\conf\gmtdefaults_SI C:\GMTdev\GMT\share\conf
	copy %GMTDIR%\share\conf\gmtdefaults_US C:\GMTdev\GMT\share\conf

	mkdir C:\GMTdev\INFO
	mkdir C:\GMTdev\INSTALLERS

	copy %GMTDIR%\guru\GMT_postinstall_message.txt C:\GMTdev\INFO
)

echo === 1. Build %BITS% GMT executables, including supplements, enabling GDAL...

set OLD_INCLUDE=%INCLUDE%
set OLD_LIB=%LIB%
set INCLUDE=%OLD_INCLUDE%;%NETCDF_DIR%\include;%GDAL_DIR%\include
set LIB=%OLD_LIB%;%NETCDF_DIR%\lib;%GDAL_DIR%\lib

cd C:\GMTdev\GMT
mkdir bin%BITS%
mkdir lib
mkdir include
cd src
call gmtinstall yes yes %BITS%
call gmtsuppl %BITS%

echo === 2. Run all the examples...

set GMT_SHAREDIR=C:\GMTdev\GMT\share
set OLDPATH=%PATH%
set PATH=C:\GMTdev\GMT\bin%BITS%;%NETCDF_DIR%\bin;%GDAL_DIR%\bin;%GNU_DIR%;%OLDPATH%

cd C:\GMTdev\GMT\doc\examples
call do_examples
cd C:\GMTdev\GMT

echo === 3. Remove all the examples PS files...

cd C:\GMTdev\GMTdoc\examples
del example_*.ps
cd C:\GMTdev\GMT

echo === 4. Build the %BITS%-bit GMT+GDAL installer...

iscc /Q %GMTDIR%\guru\GMTsetup%BITS%.iss

IF "%BITS%" == "32" (
	echo === 5. Build the GMT PDF installer...

	iscc /Q %GMTDIR%\guru\GMTsetup_pdf.iss
)

echo === 6. PLACE INSTALLERS in ftp dir

cd C:\GMTdev\
copy INSTALLERS\*.exe %GMTDIR%\ftp

set PATH=%OLDPATH%
 
echo === 7. DONE

ECHO ON
