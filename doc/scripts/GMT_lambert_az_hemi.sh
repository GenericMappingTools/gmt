#!/bin/bash
#	$Id: GMT_lambert_az_hemi.sh,v 1.9 2011-06-24 03:43:00 guru Exp $
#
. ./functions.sh

pscoast -Rg -JA280/30/3.5i -Bg -Dc -A1000 -Gnavy -P > GMT_lambert_az_hemi.ps
