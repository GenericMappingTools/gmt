REM		GMT EXAMPLE 10
REM
REM		$Id: job10.bat,v 1.12 2011-06-09 04:12:31 guru Exp $
REM
REM Purpose:	Make 3-D bar graph on top of perspective map
REM GMT progs:	pscoast, pstext, psxyz
REM DOS calls:	echo, del, gawk
REM
echo GMT EXAMPLE 10
set ps=..\example_10.ps
pscoast -Rd -JX8id/5id -Dc -Slightblue -Glightbrown -Wfaint -A1000 -p200/40 -K -U"Example 10 in Cookbook" > %ps%
psxyz agu2008.d -R-180/180/-90/90/1.01/100000 -J -JZ2.5il -So0.3ib1 -Gdarkgreen -Wthinner -B60g60/30g30/a1p:Memberships:WSneZ -O -K -p200/40 >> %ps%
echo {print $1, $2, $3} > awk.txt
gawk -f awk.txt agu2008.d | pstext -Rd -J -O -K -p200/40 -D-0.2i/0 -F+f20p,Helvetica-Bold,white=thinner+jRM >> %ps%
echo 4.5 6 AGU 2008 Membership Distribution | pstext -R0/11/0/8.5 -Jx1i -F+f30p,Times-Bold+jBC -O >> %ps%
del awk.txt .gmt*
