#!/bin/sh
#		GMT EXAMPLE 09
#
#		$Id: job09.sh,v 1.3 2003-04-11 22:57:15 pwessel Exp $
#
# Purpose:	Make wiggle plot along track from geoid deflections
# GMT progs:	pswiggle, pstext, psxy
# Unix progs:	$AWK, ls, paste, tail, rm
#
pswiggle track_*.xys -R185/250/-68/-42 -U"Example 9 in Cookbook" -K -Jm0.13i -Ba10f5 -G0 -Z2000 -W0.25p -S240/-67/500/@~m@~rad > example_09.ps
psxy -R -Jm -O -K ridge.xy -W1.75p >> example_09.ps
psxy -R -Jm -O -K -M fz.xy -W0.5pta >> example_09.ps
if [ -e ./tmp ]; then
	\rm -f tmp
fi
# Make label file
for file in track_*.xys
do
	tail -1 $file >> tmp
done
ls -1 track_*.xys | $AWK -F. '{print $2}' > tracks.lis
paste tmp tracks.lis | $AWK '{print $1, $2, 10, 50, 1, "RM", $4}' | pstext -R -Jm -D-0.05i/-0.05i -O >> example_09.ps
rm -f tmp tracks.lis .gmt*
