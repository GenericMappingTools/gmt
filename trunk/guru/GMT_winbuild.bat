ECHO OFF
REM	$Id: GMT_winbuild.bat,v 1.36 2010-07-13 00:13:19 guru Exp $
REM	Compiles GMT and builds installers under Windows.
REM	See separate GSHHS_winbuild.bat for GSHHS full+high installer
REM	Paul Wessel with help from Joaquim Luis
REM
REM	Assumptions:
REM	1. You have run make tar_all
REM	2. You have placed netcdf in C:\NETCDF
REM	4. HOME and GMTHOME has been set
REM	5. Inno Setup 5 has been installed and the path
REM	   to its command line tool is added to PATH
REM	6. 7zip has been installed and the path
REM	   to its command line tool is added to PATH

SET GVER=4.5.3
SET GSHHS=2.1.0

IF "%1%" == "home" (
	SET GMTDIR=W:\RESEARCH\PROJECTS\GMTdev\GMT
) ELSE (
SET GMTDIR=%1%:\UH\RESEARCH\PROJECTS\GMTdev\GMT
)
SET WORK=C:\WORK

echo === 1. Get all GMT%GVER% bzipped tar balls and extract files...

C:
cd %WORK%
copy %GMTDIR%\ftp\GMT%GVER%*.tar.bz2 %WORK%\
copy %GMTDIR%\ftp\GSHHS%GSHHS%_*.tar.bz2 %WORK%\
7z x GMT*.tar.bz2
7z x GSHHS*.tar.bz2
7z x GMT*.tar -aoa
7z x GSHHS*.tar -oGMT%GVER% -aoa
del *.tar.bz2
del *.tar
rename GMT%GVER% GMT
copy %GMTDIR%\src\gmt_version.h %WORK%\GMT\src
copy %GMTDIR%\src\gmt_notposix.h %WORK%\GMT\src
copy %GMTDIR%\share\conf\gmt.conf.win %WORK%\GMT\share\conf\gmt.conf
copy %GMTDIR%\share\conf\gmtdefaults_SI %WORK%\GMT\share\conf
copy %GMTDIR%\share\conf\gmtdefaults_US %WORK%\GMT\share\conf

echo === 2. Build the GMT executables, including supplements...

set INCLUDE=%INCLUDE%;C:\NETCDF\INCLUDE
set LIB=%LIB%;C:\NETCDF\LIB

cd %WORK%\GMT
mkdir bin
mkdir lib
mkdir include
cd src
call gmtinstall tri no
call gmtsuppl

echo === 3. Run all the examples...

set GMT_SHAREDIR=%WORK%\GMT\share
set OLDPATH=%PATH%
set PATH=%WORK%\GMT\bin;C:\NETCDF\bin;%OLDPATH%

cd %WORK%\GMT\share\doc\gmt\examples
call do_examples
cd %WORK%\GMT

echo === 4. Remove all the examples PS files...

cd %WORK%\GMT\share\doc\gmt\examples
for %%d in (01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30) do del ex%%d\*.ps
cd %WORK%\GMT

echo === 5. Build the GMT Basic installer...

iscc /Q %GMTDIR%\guru\GMTsetup_basic.iss

echo === 6. Rebuild the GMT executables enabling GDAL support...

cd %WORK%\GMT\src
call gmtinstall tri gdal
call gmtsuppl

echo === 7. Build the GMT+GDAL installer...

iscc /Q %GMTDIR%\guru\GMTsetup_gdal.iss

echo === 8. Build the GMT PDF installer...

iscc /Q %GMTDIR%\guru\GMTsetup_pdf.iss

echo === 9. DONE
cd %WORK%\

set PATH=%OLDPATH%

ECHO ON
