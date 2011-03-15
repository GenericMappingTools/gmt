#!/bin/bash
#	$Id: GMT_lambert_az_hemi.sh,v 1.5 2011-03-15 02:06:29 guru Exp $
#
. functions.sh

pscoast -Rg -JA280/30/3.5i -B30g30/15g15 -Dc -A1000 -Gnavy -P > GMT_lambert_az_hemi.ps
