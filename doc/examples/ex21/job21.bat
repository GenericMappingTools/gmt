REM
REM		GMT EXAMPLE 21
REM
REM		$Id: job21.bat,v 1.1 2004-04-23 22:50:42 pwessel Exp $
REM
REM Purpose:	
REM GMT progs:	
REM DOS calls:	
REM
echo GMT EXAMPLE 21
set master=y
if exist job21.bat set master=n
if %master%==y cd ex21
psbasemap -R0/6.5/0/9 -Jx1i -B0 -P -U"Example 21 in Cookbook" > example_21.ps
del .gmt*
if %master%==y cd ..
