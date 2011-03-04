ECHO OFF
REM	$Id: GSHHS_winbuild.bat,v 1.14 2011-03-04 06:28:49 guru Exp $
REM	Builds installer for GSHHS under Windows
REM	Paul Wessel with help from Joaquim Luis
REM
REM	Assumptions:
REM	1. You have run make tar_high tar_full
REM	2. Inno Setup 5 has been installed and the path
REM	   to its command line tool is added to PATH
REM	3. 7zip has been installed and the path
REM	   to its command line tool is added to PATH

SET GSHHS=2.1.1

IF "%1%" == "home" (
	SET GMTDIR=W:\RESEARCH\PROJECTS\GMTdev\GMT4
) ELSE (
	SET GMTDIR=%1%:\UH\RESEARCH\PROJECTS\GMTdev\GMT4
)

echo === 1. Get all GSHHS %GSHHS% bzipped tar balls and extract files...

C:
cd C:\GMTdev
copy %GMTDIR%\ftp\GSHHS%GSHHS%_high.tar.bz2 C:\GMTdev\
copy %GMTDIR%\ftp\GSHHS%GSHHS%_full.tar.bz2 C:\GMTdev\
7z x GSHHS%GSHHS%_high.tar.bz2
7z x GSHHS%GSHHS%_full.tar.bz2
7z x GSHHS%GSHHS%_high.tar -oGMT -aoa
7z x GSHHS%GSHHS%_full.tar -oGMT -aoa
del GSHHS%GSHHS%_*.tar.bz2
del GSHHS%GSHHS%_*.tar

echo === 2. Build the GSHHS %GSHHS% full/high installer...

iscc /Q %GMTDIR%\guru\GMTsetup_hfcoast.iss

copy INSTALLERS\*.exe %GMTDIR%\ftp

echo === 4. DONE
ECHO ON
