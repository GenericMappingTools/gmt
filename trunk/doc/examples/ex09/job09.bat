REM		GMT EXAMPLE 09
REM
REM		$Id: job09.bat,v 1.1.1.1 2000-12-28 01:23:45 gmt Exp $
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
pswiggle all.xys -R185/250/-68/-42 -U"Example 9 in Cookbook" -K -Jm0.13i -Ba10f5 -G0 -Z2000 -W0.25p -M -S240/-67/500/@~m@~rad > example_09.ps
psxy -R -Jm -O -K ridge.xy -W1.75p >> example_09.ps
psxy -R -Jm -O -K -M fz.xy -W0.5pta >> example_09.ps
REM Plot labels
gawk "{if (NF == 5) print $3, $4, 10, 50, 1, 7, $2}" all.xys | pstext -R -Jm -D-0.05i/-0.05i -O >> example_09.ps
del .gmt*
if %master%==y cd ..
