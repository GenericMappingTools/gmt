#!/bin/bash
#	$Id: GMT_eckert4.sh,v 1.6 2011-02-28 00:58:03 remko Exp $
#
. functions.sh

pscoast -Rg -JKf4.5i -Bg30/g15 -Dc -A10000 -Wthinnest -Gwhite -Slightgray -P > GMT_eckert4.ps
