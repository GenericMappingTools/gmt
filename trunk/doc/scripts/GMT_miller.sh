#!/bin/sh
#	$Id: GMT_miller.sh,v 1.2 2004-04-10 17:19:14 pwessel Exp $
#

pscoast -R-90/270/-80/90 -Jj90/1:400000000 -B45g45/30g30 -Dc -A10000 -Glightgray -W0.25p -P \
    > GMT_miller.ps
