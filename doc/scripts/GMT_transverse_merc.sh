#!/bin/bash
#	$Id: GMT_transverse_merc.sh,v 1.7 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh

pscoast -R20/30/50/45r -Jt35/0.18i -B10g5 -Dl -A250 -Glightgray -Wthinnest -P \
	> GMT_transverse_merc.ps
