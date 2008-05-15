ECHO OFF
REM	$Id: GSHHS_winbuild.bat,v 1.1 2008-05-15 03:26:14 guru Exp $
REM	Builds installer for GSHHS under Windows
REM	Paul Wessel with help from Joaquim Luis
REM
REM	Assumptions:
REM	1. You have placed the GMT distribution in C:\GMT
REM	   Do make zip_dist to get a single GMT_dist.zip
REM	2. Inno Setup 5 has been installed and the path
REM	   to its command line tool is added to PATH


echo === 1. Build the GSHHS full/high installer...
iscc /Q C:\GMT\guru\GMTsetup_hfcoast.iss
echo === GSHHS installer is now placed in C:\GMT\ftp.
echo === Copy it over to your GMT/ftp on macnut.
ECHO ON
