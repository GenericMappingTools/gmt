#!/bin/sh
#	$Id: GMT_volcano.sh,v 1.3 2002-04-09 18:18:41 pwessel Exp $

echo "0 0" | psxy -R-0.5/0.5/-0.5/0.5 -JX2i -P -Ba0.25g0.05WSne -W1p -Skvolcano/2i > GMT_volcano.ps
