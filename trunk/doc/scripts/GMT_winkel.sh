#!/bin/bash
#	$Id: GMT_winkel.sh,v 1.7 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh

pscoast -Rd -JR4.5i -Bg30/g15 -Dc -A10000 -Ggray -P > GMT_winkel.ps
