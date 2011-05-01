#!/bin/bash
#	$Id: GMT_eckert4.sh,v 1.8 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh

pscoast -Rg -JKf4.5i -Bg30/g15 -Dc -A10000 -Wthinnest -Gwhite -Slightgray -P > GMT_eckert4.ps
