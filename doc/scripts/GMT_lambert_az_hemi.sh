#!/bin/bash
#	$Id: GMT_lambert_az_hemi.sh,v 1.4 2011-02-28 00:58:01 remko Exp $
#
. functions.sh

pscoast -Rg -JA280/30/3.5i -B30g30/15g15 -Dc -A1000 -Gblack -P > GMT_lambert_az_hemi.ps
