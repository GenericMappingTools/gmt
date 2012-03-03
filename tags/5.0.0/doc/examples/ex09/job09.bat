REM		GMT EXAMPLE 09
REM
REM		$Id$
REM
REM Purpose:	Make wiggle plot along track from geoid deflections
REM GMT progs:	pswiggle, pstext, psxy
REM DOS calls:	gawk, del
REM Remark:	Differs from Unix versions in that a single multisegment file
REM		is used, with precomputed annotation info stored in the headers.
REM		See the Unix version for how computations are done.
REM
echo GMT EXAMPLE 09
set ps=..\example_09.ps
pswiggle all.xys -R185/250/-68/-42 -U"Example 9 in Cookbook" -K -Jm0.13i -Ba10f5WEsn+g240/255/240 -G+red -G-blue -Z2000 -Wthinnest -S240/-67/500/@~m@~rad > %ps%
psxy -R -J -O -K ridge.xy -Wthicker >> %ps%
psxy -R -J -O -K fz.xy -Wthinner,- >> %ps%
REM Plot labels
gawk "{if (NF == 5) print $3, $4, $2}" all.xys | pstext -R -J -F+f10p,Helvetica-Bold+a50+jRM -D-0.05i/-0.05i -O >> %ps%
del .gmt*
