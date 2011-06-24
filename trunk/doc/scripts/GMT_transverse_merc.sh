#!/bin/bash
#	$Id: GMT_transverse_merc.sh,v 1.9 2011-06-24 21:13:45 guru Exp $
#
. ./functions.sh

pscoast -R20/30/50/45r -Jt35/0.18i -Bag -Dl -A250 -Glightbrown -Wthinnest -P \
	-Sseashell > GMT_transverse_merc.ps
