REM		GMT EXAMPLE 10
REM
REM		$Id: job10.bat,v 1.10 2011-03-01 01:34:48 remko Exp $
REM
REM Purpose:	Make 3-D bar graph on top of perspective map
REM GMT progs:	pscoast, pstext, psxyz
REM DOS calls:	echo, del, gawk
REM
echo GMT EXAMPLE 10
set master=y
if exist job10.bat set master=n
if %master%==y cd ex10
pscoast -Rd -JX8id/5id -Dc -Gblack -E200/40 -K -U"Example 10 in Cookbook" > ..\example_10.ps
psxyz agu2008.d -R-180/180/-90/90/1/100000 -J -JZ2.5il -So0.3ib1 -Ggray -Wthinner -B60g60/30g30/a1p:Memberships:WSneZ -O -K -E200/40 >> ..\example_10.ps
gawk "{print $1, $2, 20, 0, 0, 7, $3}" agu2008.d | pstext -Rd -J -O -K -E200/40 -Gwhite -Sthinner -D-0.2i/0 >> ..\example_10.ps
echo 4.5 6 30 0 5 2 AGU 2008 Membership Distribution | pstext -R0/11/0/8.5 -Jx1i -O >> ..\example_10.ps
del .gmt*
if %master%==y cd ..
