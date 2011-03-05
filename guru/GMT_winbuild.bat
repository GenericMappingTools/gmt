ECHO OFF
REM	$Id: GMT_winbuild.bat,v 1.47 2011-03-05 19:48:58 guru Exp $
REM	Compiles GMT and builds installers under Windows.
REM	See separate GSHHS_winbuild.bat for GSHHS full+high installer
REM	Paul Wessel with help from Joaquim Luis
REM
REM	Assumptions:
REM	1. You have run make tar_all tar_coast
REM	2. You have placed netcdf in C:\NETCDF
REM	3. You have C:\GMTdev with dirs INFO and INSTALLERS
REM	4. Inno Setup 5 has been installed and the path
REM	   to its command line tool is added to PATH
REM	5. 7zip has been installed and the path
REM	   to its command line tool is added to PATH
REM	6. FWTools2.4.7 has been installed under C:\programs
REM	   and the path to its DLL is added to PATH

SET GVER=4.5.6
SET GSHHS=2.1.1

IF "%1%" == "home" (
	SET GMTDIR=W:\RESEARCH\PROJECTS\GMTdev\GMT4
) ELSE (
	SET GMTDIR=%1%:\UH\RESEARCH\PROJECTS\GMTdev\GMT4
)

echo === 1. Get all GMT%GVER% bzipped tar balls and extract files...

C:
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

echo === 2. Build 32 GMT executables, including supplements, enabling GDAL...

set OLD_INCLUDE=%INCLUDE%
set OLD_LIB=%LIB%
set INCLUDE=%OLD_INCLUDE%;C:\GMTdev\netcdf-3.6.3\VC10_32\include;C:\GMTdev\gdal\VC10_32\include
set LIB=%OLD_LIB%;C:\GMTdev\netcdf-3.6.3\VC10_32\lib;C:\GMTdev\gdal\VC10_32\lib

cd C:\GMTdev\GMT
mkdir bin32
mkdir lib
mkdir include
cd src
call gmtinstall yes yes 32
call gmtsuppl 32

echo === 3. Run all the examples...

set GMT_SHAREDIR=C:\GMTdev\GMT\share
set OLDPATH=%PATH%
set PATH=C:\GMTdev\GMT\bin;C:\NETCDF\bin;%OLDPATH%

cd C:\GMTdev\GMT\share\doc\gmt\examples
call do_examples
cd C:\GMTdev\GMT

echo === 4. Remove all the examples PS files...

cd C:\GMTdev\GMT\share\doc\gmt\examples
for %%d in (01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30) do del ex%%d\*.ps
cd C:\GMTdev\GMT

echo === 5. Build the 32-bit GMT+GDAL installer...

iscc /Q %GMTDIR%\guru\GMTsetup32.iss

echo === 6. Build 64 GMT executables, including supplements, enabling GDAL...

set INCLUDE=%OLD_INCLUDE%;C:\GMTdev\netcdf-3.6.3\VC10_64\include;C:\GMTdev\gdal\VC10_64\include
set LIB=%OLD_LIBLIB%;C:\GMTdev\netcdf-3.6.3\VC10_64\lib;C:\GMTdev\gdal\VC10_64\lib

cd C:\GMTdev\GMT
mkdir bin64
cd src
call gmtinstall yes yes 64
call gmtsuppl 32

echo === 7. Run all the examples...

set GMT_SHAREDIR=C:\GMTdev\GMT\share
set OLDPATH=%PATH%
set PATH=C:\GMTdev\GMT\bin;C:\NETCDF\bin;%OLDPATH%

cd C:\GMTdev\GMT\share\doc\gmt\examples
call do_examples
cd C:\GMTdev\GMT

echo === 8. Remove all the examples PS files...

cd C:\GMTdev\GMT\share\doc\gmt\examples
for %%d in (01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30) do del ex%%d\*.ps
cd C:\GMTdev\GMT

echo === 5. Build the 64-bit GMT+GDAL installer...

iscc /Q %GMTDIR%\guru\GMTsetup64.iss

echo === 6. Build the GMT PDF installer...

iscc /Q %GMTDIR%\guru\GMTsetup_pdf.iss

echo === 7. PLACE INSTALLERS in ftp dir

cd C:\GMTdev\
copy INSTALLERS\*.exe %GMTDIR%\ftp

set PATH=%OLDPATH%
 
echo === 8. DONE

ECHO ON
