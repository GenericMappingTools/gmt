#!/bin/bash
#	$Id: GMT_TM.sh,v 1.6 2011-05-22 22:14:05 guru Exp $
#
. ./functions.sh

pscoast -R0/360/-80/80 -JT330/-45/3.5i -B30g15/15g15WSne -Dc -A2000 -Slightblue -G0 -P > GMT_TM.ps
