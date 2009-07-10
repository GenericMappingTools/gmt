ECHO OFF
REM	$Id: GSHHS_winbuild.bat,v 1.6 2009-07-10 00:09:33 guru Exp $
REM	Builds installer for GSHHS under Windows
REM	Paul Wessel with help from Joaquim Luis
REM
REM	Assumptions:
REM	1. You have run make tar_high tar_full
REM	2. Inno Setup 5 has been installed and the path
REM	   to its command line tool is added to PATH
REM	3. 7zip has been installed and the path
REM	   to its command line tool is added to PATH

SET GSHHS=2.0

echo === 1. Get all GSHHS %GSHHS% bzipped tar balls and extract files...

C:
cd \
copy Y:\UH\RESEARCH\PROJECTS\GMTdev\GMT\ftp\GSHHS*high.tar.bz2 C:\
copy Y:\UH\RESEARCH\PROJECTS\GMTdev\GMT\ftp\GSHHS*full.tar.bz2 C:\
7z x GSHHS*high.tar.bz2
7z x GSHHS*full.tar.bz2
7z x GSHHS*high.tar -oGMT -aoa
7z x GSHHS*full.tar -oGMT -aoa
del GSHHS*.tar.bz2
del GSHHS*.tar

echo === 2. Build the GSHHS full/high installer...

iscc /Q Y:\UH\RESEARCH\PROJECTS\GMTdev\GMT\guru\GMTsetup_hfcoast.iss

echo === 4. DONE
ECHO ON
