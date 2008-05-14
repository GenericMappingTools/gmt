ECHO OFF
REM	$Id: GMT_winbuild.bat,v 1.4 2008-05-14 00:48:57 guru Exp $
REM	Compiles GMT and builds installers under Windows
REM	Paul Wessel with help from Joaquim Luis
REM
REM	Assumptions:
REM	1. You have placed the GMT distribution in C:\GMT
REM	   Do make zip_dist to get a single GMT_dist.zip
REM	2. You have placed netcdf in C:\NETCDF
REM	3. INCLUDE, LIB, PATH have been set so that CL and
REM	   LIB will find the netcdf include and library
REM	4. HOME and GMTHOME has been set
REM	5. Inno Setup 5 has been installed and the path
REM	   to its command line tool is added to PATH


echo "1. Build the GMT executables, including supplements..."
cd C:\GMT
mkdir bin
mkdir lib
cd src
call gmtinstall tri
call gmtsuppl
echo "2. Build the GMT Basic installer..."
iscc /Q C:\GMT\guru\GMTsetup_basic.iss
echo "3. Build the GMT PDF installer..."
iscc /Q C:\GMT\guru\GMTsetup_pdf.iss
echo "4. Build the GSHHS full/high installer..."
iscc /Q C:\GMT\guru\GMTsetup_hfcoast.iss
echo GMT installers are now placed in C:\GMT\ftp.
echo Copy them over to your GMT/ftp on macnut.
