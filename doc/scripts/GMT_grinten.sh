#!/bin/bash
#	$Id: GMT_grinten.sh,v 1.8 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh

pscoast -Rg -JV4i -Bg30/g15 -Dc -Glightgray -A10000 -Wthinnest -P > GMT_grinten.ps
