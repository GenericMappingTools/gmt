#!/bin/bash
#	$Id: GMT_transverse_merc.sh,v 1.8 2011-05-22 22:14:06 guru Exp $
#
. ./functions.sh

pscoast -R20/30/50/45r -Jt35/0.18i -B10g5 -Dl -A250 -Glightbrown -Wthinnest -P \
	-Sseashell > GMT_transverse_merc.ps
