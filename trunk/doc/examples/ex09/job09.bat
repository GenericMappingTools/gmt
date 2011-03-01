REM		GMT EXAMPLE 09
REM
REM		$Id: job09.bat,v 1.8 2011-03-01 01:34:48 remko Exp $
REM
REM Purpose:	Make wiggle plot along track from geoid deflections
REM GMT progs:	pswiggle, pstext, psxy
REM DOS calls:	gawk, del
REM Remark:	Differs from Unix versions in that a single multisegment file
REM		is used, with anotation info stored in the headers.
REM
echo GMT EXAMPLE 09
set master=y
if exist job09.bat set master=n
if %master%==y cd ex09
pswiggle all.xys -R185/250/-68/-42 -U"Example 9 in Cookbook" -K -Jm0.13i -Ba10f5 -Gblack -Z2000 -Wthinnest -m -S240/-67/500/@~m@~rad > ..\example_09.ps
psxy -R -J -O -K ridge.xy -Wthicker >> ..\example_09.ps
psxy -R -J -O -K -m fz.xy -Wthinner,- >> ..\example_09.ps
REM Plot labels
gawk "{if (NF == 5) print $3, $4, 10, 50, 1, 7, $2}" all.xys | pstext -R -J -D-0.05i/-0.05i -O >> ..\example_09.ps
del .gmt*
if %master%==y cd ..
