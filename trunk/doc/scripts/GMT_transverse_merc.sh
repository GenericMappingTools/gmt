#!/bin/bash
#	$Id: GMT_transverse_merc.sh,v 1.5 2011-02-28 00:58:03 remko Exp $
#
. functions.sh

pscoast -R20/30/50/45r -Jt35/0.18i -B10g5 -Dl -A250 -Glightgray -Wthinnest -P \
	> GMT_transverse_merc.ps
