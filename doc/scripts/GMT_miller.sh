#!/bin/sh
#	$Id: GMT_miller.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

pscoast -R-90/270/-80/90 -Jj90/1:400000000 -B45g45/30g30 -Dc -A10000 -G200 -W0.25p -P \
    > GMT_miller.ps
