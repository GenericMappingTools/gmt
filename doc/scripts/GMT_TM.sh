#!/bin/bash
#	$Id: GMT_TM.sh,v 1.4 2011-03-15 02:06:29 guru Exp $
#
. functions.sh

pscoast -R0/360/-80/80 -JT330/-45/3.5i -B30g15/15g15WSne -Dc -A2000 -Gblack -P > GMT_TM.ps
