REM
REM		GMT EXAMPLE 25
REM
REM		$Id: job25.bat,v 1.1 2004-04-23 22:50:42 pwessel Exp $
REM
REM Purpose:	
REM GMT progs:	
REM DOS calls:	
REM
echo GMT EXAMPLE 25
set master=y
if exist job25.bat set master=n
if %master%==y cd ex25
psbasemap -R0/6.5/0/9 -Jx1i -B0 -P -U"Example 25 in Cookbook" > example_25.ps
del .gmt*
if %master%==y cd ..
