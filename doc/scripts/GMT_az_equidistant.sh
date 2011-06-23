#!/bin/bash
#	$Id: GMT_az_equidistant.sh,v 1.9 2011-06-23 01:17:06 remko Exp $
#
. ./functions.sh

pscoast -Rg -JE-100/40/4.5i -B15g15 -Dc -A10000 -Glightgray -Wthinnest -P > GMT_az_equidistant.ps
