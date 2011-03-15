#!/bin/bash
#	$Id: GMT_volcano.sh,v 1.8 2011-03-15 02:06:29 guru Exp $
. functions.sh

ln -s ../../share/doc/gmt/examples/ex20/bullseye.def .
echo "0 0" | psxy -R-0.5/0.5/-0.5/0.5 -JX2i -P -Ba0.25g0.05WSne -Wthick -Skvolcano/2i -K > GMT_volcano.ps
echo "0 0" | psxy -R -J -N -Ba0.25g0.05wSnE -Wthick -Skbullseye/2i -O -X2.5i >> GMT_volcano.ps
rm -f bullseye.def
