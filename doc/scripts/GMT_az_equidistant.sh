#!/bin/bash
#	$Id: GMT_az_equidistant.sh,v 1.6 2011-03-15 02:06:29 guru Exp $
#
. functions.sh

pscoast -Rg -JE-100/40/4.5i -B15g15 -Dc -A10000 -Glightgray -Wthinnest -P > GMT_az_equidistant.ps
