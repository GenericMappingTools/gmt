REM
REM		GMT EXAMPLE 23
REM
REM		$Id: job23.bat,v 1.1 2004-04-23 22:50:42 pwessel Exp $
REM
REM Purpose:	
REM GMT progs:	
REM DOS calls:	
REM
echo GMT EXAMPLE 23
set master=y
if exist job23.bat set master=n
if %master%==y cd ex23
psbasemap -R0/6.5/0/9 -Jx1i -B0 -P -U"Example 23 in Cookbook" > example_23.ps
del .gmt*
if %master%==y cd ..
