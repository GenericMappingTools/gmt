ECHO OFF
REM	$ID$
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
iscc C:\GMT\guru\GMTsetup_basic.iss
iscc C:\GMT\guru\GMTsetup_hfcoast.iss
iscc C:\GMT\guru\GMTsetup_pdf.iss
echo "GMT installers are now placed in C:\GMT\ftp"
echo "copy them over to your GMT/ftp on macnut"
