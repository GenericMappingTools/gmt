#!/bin/bash
#	$Id: GMT_lambert_az_hemi.sh,v 1.7 2011-06-22 00:35:06 remko Exp $
#
. ./functions.sh

pscoast -Rg -JA280/30/3.5i -B30g/15g -Dc -A1000 -Gnavy -P > GMT_lambert_az_hemi.ps
