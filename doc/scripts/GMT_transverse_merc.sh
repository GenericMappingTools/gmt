#!/bin/sh
#	$Id: GMT_transverse_merc.sh,v 1.2 2004-04-10 17:19:14 pwessel Exp $
#

pscoast -R20/30/50/45r -Jt35/0.18i -B10g5 -Dl -A250 -Glightgray -W0.25p -P > GMT_transverse_merc.ps
