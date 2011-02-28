#!/bin/bash
#	$Id: GMT_TM.sh,v 1.3 2011-02-28 00:58:00 remko Exp $
#
. functions.sh

pscoast -R0/360/-80/80 -JT330/-45/3.5i -B30g15/15g15WSne -Dc -A2000 -Gblack -P > GMT_TM.ps
