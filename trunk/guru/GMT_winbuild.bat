ECHO OFF
REM	$Id: GMT_winbuild.bat,v 1.17 2009-01-16 01:36:56 guru Exp $
REM	Compiles GMT and builds installers under Windows.
REM	See separate GSHHS_winbuild.bat for GSHHS full+high installer
REM	Paul Wessel with help from Joaquim Luis
REM
REM	Assumptions:
REM	1. You have run make tar_all
REM	2. You have placed netcdf in C:\NETCDF
REM	3. INCLUDE, LIB, PATH have been set so that CL and
REM	   LIB will find the netcdf include and library
REM	4. HOME and GMTHOME has been set
REM	5. Inno Setup 5 has been installed and the path
REM	   to its command line tool is added to PATH
REM	6. 7zip has been installed and the path
REM	   to its command line tool is added to PATH

SET GVER=4.4.0

echo === 1. Get all GMT%GVER% bzipped tar balls and extract files...

cd C:\
copy Y:\UH\RESEARCH\PROJECTS\GMTdev\GMT\ftp\GMT*.tar.bz2 C:\
copy Y:\UH\RESEARCH\PROJECTS\GMTdev\GMT\ftp\GSHHS*.tar.bz2 C:\
7z x GMT*.tar.bz2
7z x GSHHS*.tar.bz2
7z x GMT*.tar -aoa
7z x GSHHS*.tar -oGMT%GVER% -aoa
del *.tar.bz2
del *.tar
rename GMT%GVER% GMT
copy Y:\UH\RESEARCH\PROJECTS\GMTdev\GMT\src\gmt_version.h C:\GMT\src
copy Y:\UH\RESEARCH\PROJECTS\GMTdev\GMT\src\gmt_notposix.h C:\GMT\src
copy Y:\UH\RESEARCH\PROJECTS\GMTdev\GMT\share\conf\gmt.conf C:\GMT\share\conf
copy Y:\UH\RESEARCH\PROJECTS\GMTdev\GMT\share\conf\gmtdefaults_SI C:\GMT\share\conf
copy Y:\UH\RESEARCH\PROJECTS\GMTdev\GMT\share\conf\gmtdefaults_US C:\GMT\share\conf

echo === 2. Build the GMT executables, including supplements...

cd C:\GMT
mkdir bin
mkdir lib
mkdir include
cd src
call gmtinstall tri
call gmtsuppl

echo === 3. Run all the examples...

cd C:\GMT\examples
call do_examples
cd C:\GMT

echo === 4. Build the GMT Basic installer...

iscc /Q Y:\UH\RESEARCH\PROJECTS\GMTdev\GMT\guru\GMTsetup_basic.iss

echo === 5. Build the GMT PDF installer...

iscc /Q Y:\UH\RESEARCH\PROJECTS\GMTdev\GMT\guru\GMTsetup_pdf.iss

echo === 6. DONE
cd C:\

ECHO ON
