#!/bin/bash
#	$Id: GMT_gnomonic.sh,v 1.8 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh

pscoast -Rg -JF-120/35/60/4.5i -B30g15 -Dc -A10000 -Gtan -Scyan -Wthinnest -P > GMT_gnomonic.ps
