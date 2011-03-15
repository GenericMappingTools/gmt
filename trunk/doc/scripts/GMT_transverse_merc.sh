#!/bin/bash
#	$Id: GMT_transverse_merc.sh,v 1.6 2011-03-15 02:06:29 guru Exp $
#
. functions.sh

pscoast -R20/30/50/45r -Jt35/0.18i -B10g5 -Dl -A250 -Glightgray -Wthinnest -P \
	> GMT_transverse_merc.ps
