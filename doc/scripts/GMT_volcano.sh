#!/bin/sh
#	$Id: GMT_volcano.sh,v 1.2 2001-09-23 23:41:15 pwessel Exp $

echo "0 0" | psxy -R-0.5/0.5/-0.5/0.5 -JX2i -P -Ba0.25g0.05WSne -W1p -Skvolcano/1i > GMT_volcano.ps
