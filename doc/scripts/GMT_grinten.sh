#!/bin/bash
#	$Id: GMT_grinten.sh,v 1.6 2011-02-28 00:58:03 remko Exp $
#
. functions.sh

pscoast -Rg -JV4i -Bg30/g15 -Dc -Glightgray -A10000 -Wthinnest -P > GMT_grinten.ps
