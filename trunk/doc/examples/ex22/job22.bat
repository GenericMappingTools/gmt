REM
REM		GMT EXAMPLE 22
REM
REM		$Id: job22.bat,v 1.1 2004-04-23 22:50:42 pwessel Exp $
REM
REM Purpose:	
REM GMT progs:	
REM DOS calls:	
REM
echo GMT EXAMPLE 22
set master=y
if exist job22.bat set master=n
if %master%==y cd ex22
psbasemap -R0/6.5/0/9 -Jx1i -B0 -P -U"Example 22 in Cookbook" > example_22.ps
del .gmt*
if %master%==y cd ..
