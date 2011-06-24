#!/bin/bash
#	$Id: GMT_az_equidistant.sh,v 1.10 2011-06-24 03:43:00 guru Exp $
#
. ./functions.sh

pscoast -Rg -JE-100/40/4.5i -Bg -Dc -A10000 -Glightgray -Wthinnest -P > GMT_az_equidistant.ps
