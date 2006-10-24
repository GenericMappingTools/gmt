#!/bin/sh
#	$Id: GMT_volcano.sh,v 1.4 2006-10-24 01:53:19 remko Exp $

echo "0 0" | psxy -R-0.5/0.5/-0.5/0.5 -JX2i -P -Ba0.25g0.05WSne -Wthick -Skvolcano/2i > GMT_volcano.ps
