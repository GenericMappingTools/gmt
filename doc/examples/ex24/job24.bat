REM
REM		GMT EXAMPLE 24
REM
REM		$Id: job24.bat,v 1.1 2004-04-23 22:50:42 pwessel Exp $
REM
REM Purpose:	
REM GMT progs:	
REM DOS calls:	
REM
echo GMT EXAMPLE 24
set master=y
if exist job24.bat set master=n
if %master%==y cd ex24
psbasemap -R0/6.5/0/9 -Jx1i -B0 -P -U"Example 24 in Cookbook" > example_24.ps
del .gmt*
if %master%==y cd ..
