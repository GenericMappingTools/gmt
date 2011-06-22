#!/bin/bash
#	$Id: GMT_az_equidistant.sh,v 1.8 2011-06-22 00:35:06 remko Exp $
#
. ./functions.sh

pscoast -Rg -JE-100/40/4.5i -B15g -Dc -A10000 -Glightgray -Wthinnest -P > GMT_az_equidistant.ps
