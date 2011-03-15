#!/bin/bash
#	$Id: GMT_gnomonic.sh,v 1.7 2011-03-15 02:06:29 guru Exp $
#
. functions.sh

pscoast -Rg -JF-120/35/60/4.5i -B30g15 -Dc -A10000 -Gtan -Scyan -Wthinnest -P > GMT_gnomonic.ps
