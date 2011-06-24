#!/bin/bash
#	$Id: GMT_TM.sh,v 1.7 2011-06-24 21:13:45 guru Exp $
#
. ./functions.sh

pscoast -R0/360/-80/80 -JT330/-45/3.5i -Ba30gWSne -Dc -A2000 -Slightblue -G0 -P > GMT_TM.ps
