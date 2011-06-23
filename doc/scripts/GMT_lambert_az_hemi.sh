#!/bin/bash
#	$Id: GMT_lambert_az_hemi.sh,v 1.8 2011-06-23 01:17:06 remko Exp $
#
. ./functions.sh

pscoast -Rg -JA280/30/3.5i -B30g30/15g15 -Dc -A1000 -Gnavy -P > GMT_lambert_az_hemi.ps
