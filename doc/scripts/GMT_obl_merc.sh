#!/bin/sh
#	$Id: GMT_obl_merc.sh,v 1.3 2004-04-13 21:32:27 pwessel Exp $
#

pscoast -R270/20/305/25r -JOc280/25.5/22/69/4.8i -B10g5 -Dl -A250 -Glightgray -W0.25p -P > \
	GMT_obl_merc.ps
