#!/bin/bash
#	$Id: GMT_grinten.sh,v 1.7 2011-03-15 02:06:29 guru Exp $
#
. functions.sh

pscoast -Rg -JV4i -Bg30/g15 -Dc -Glightgray -A10000 -Wthinnest -P > GMT_grinten.ps
