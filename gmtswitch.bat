@ECHO OFF
REM  
REM	$Id: gmtswitch.bat,v 1.1 2011-06-25 20:37:58 jluis Exp $
REM
REM	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
REM	See LICENSE.TXT file for copying and redistribution conditions.
REM
REM	This program is free software; you can redistribute it and/or modify
REM	it under the terms of the GNU General Public License as published by
REM	the Free Software Foundation; version 2 of the License.
REM
REM	This program is distributed in the hope that it will be useful,
REM	but WITHOUT ANY WARRANTY; without even the implied warranty of
REM	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
REM	GNU General Public License for more details.
REM
REM	Contact info: gmt.soest.hawaii.edu
REM -------------------------------------------------------------------------------
REM	gmtswitch - switch between several installed GMT versions
REM
REM	This batch changes the Windows PATH variable so that the BIN directory
REM	of the preferred version always comes first. To do that the batch works in two
REM	alternative modes.
REM
REM	1- Permanent mode
REM	2- Temporary mode
REM
REM	The permanent mode makes use of the free executable program "EditPath" to
REM	change the user path in the registry. It's called permanent because the changes
REM	remains until ... next change.
REM	http://www.softpedia.com/get/Tweak/Registry-Tweak/EditPath.shtml 
REM	Off course the editpath.exe binary must be in your system's path as well.
REM
REM	The second mode is temporary because the path to the selected GMT binary dir is
REM	prepended to the via a shell command line. This modification disappears when the
REM	shell cmd window where it was executes is deleted.
REM
REM	It is the user responsibility to set the contents of the G32_32 to G5_64 bellow
REM	to valid paths where the binaries of the different GMT versions are installed
REM	Note that it's not mandatory to have all four of them in you computer. For the
REM	ones you do not have just let them pointing to nothing e.g.
REM	set G4_64=
REM
REM	The permanent mode is the default one (but this can be changed. See edit section)
REM	To run in the temporary mode just give a second argument (doesn't matter what)
REM
REM	Example usage to set a GMT5 64 bits permanent
REM	gmtswitch g5_64
REM
REM	To temporary set a GMT4 32 bits do
REM	gmtswitch g4_32 1
REM
REM	Run without arguments to get a "Usage" (for permanent mode)


REM -------- Make this correct in your system ------------------
set G4_32=C:\programs\GMT\GMT_win32\bin
set G4_64=C:\programs\GMT\GMT_win64\bin
set G5_32=C:\programs\GMTdev\GMT5\WIN32\bin
set G5_64=C:\programs\GMTdev\GMT5\WIN64\bin

REM Set next variable to 0 if you always want to run this batch in the temporary mode.
set PERMANENT=1

REM ------------- STOP EDITING HERE -----------------------------

IF EXIST %2% == "" (
set PERMANENT=0
)


IF "%1%"=="g5_32" (
IF %PERMANENT% == 0 (
set pato=%G5_32%;
GOTO FCRAZY_INSIDE_IF_DOES_NOT_WORK
) ELSE (
editpath -u -q -r %G4_32%
editpath -u -q -r %G4_64%
editpath -u -q -r %G5_64%
editpath -u -q -a -b %G5_32%
)
GOTO FIM
)

IF "%1%"=="g5_64" (
IF %PERMANENT% == 0 (
set pato=%G5_64%;
GOTO FCRAZY_INSIDE_IF_DOES_NOT_WORK
) ELSE (
editpath -u -q -r %G4_32%
editpath -u -q -r %G4_64%
editpath -u -q -r %G5_32%
editpath -u -q -a -b %G5_64%
)
GOTO FIM
)

IF "%1%"=="g4_64" (
IF %PERMANENT% == 0 (
set pato=%G4_64%
GOTO FCRAZY_INSIDE_IF_DOES_NOT_WORK
) ELSE (
editpath -u -q -r %G4_32%
editpath -u -q -r %G5_64%
editpath -u -q -r %G5_32%
editpath -u -q -a -b %G4_64%
)
GOTO FIM
)

IF "%1%"=="g4_32" (
IF %PERMANENT% == 0 (
set pato=%G4_32%;
GOTO FCRAZY_INSIDE_IF_DOES_NOT_WORK
) ELSE (
editpath -u -q -r %G4_64%
editpath -u -q -r %G5_64%
editpath -u -q -r %G5_32%
editpath -u -q -a -b %G4_32%
)
GOTO FIM
)

echo Usage:
IF NOT "%G5_64%" == "" (
echo 	To set GMT5 64 bits installed at %G5_64% run
echo gmtswitch g5_64
)
IF NOT "%G5_32%" == "" (
echo 	To set GMT5 32 bits installed at %G5_32% run
echo gmtswitch g5_32
)
IF NOT "%G4_64%" == "" (
echo 	To set GMT5 64 bits installed at %G4_64% run
echo gmtswitch g4_64
)
IF NOT "%G4_32%" == "" (
echo 	To set GMT5 32 bits installed at %G4_32% run
echo gmtswitch g4_32
)
GOTO FIM

:FCRAZY_INSIDE_IF_DOES_NOT_WORK
REM Land here when we want prepend to path that.
REM It's unbelievable but next instruction completely screws up when executed 
REM inside an IF test because of the bloody blanks in path names (e.g. Program Files)
set path=%pato%;%path%

:FIM
