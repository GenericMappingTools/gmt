#!/bin/bash
#	$Id: GMT_gnomonic.sh,v 1.6 2011-02-28 00:58:03 remko Exp $
#
. functions.sh

pscoast -Rg -JF-120/35/60/4.5i -B30g15 -Dc -A10000 -Glightgray -Wthinnest -P > GMT_gnomonic.ps
