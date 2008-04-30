ECHO OFF
REM	$Id: GMT_winbuild.bat,v 1.2 2008-04-30 21:07:17 guru Exp $
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


cd C:\GMT
mkdir bin
mkdir lib
cd src
gmtinstall tri
gmtsuppl
iscc /Q C:\GMT\guru\GMTsetup_basic.iss
iscc /Q C:\GMT\guru\GMTsetup_hfcoast.iss
iscc /Q C:\GMT\guru\GMTsetup_pdf.iss
echo "GMT installers are now placed in C:\GMT\ftp"
echo "copy them over to your GMT/ftp on macnut"
