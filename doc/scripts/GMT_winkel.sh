#!/bin/bash
#	$Id: GMT_winkel.sh,v 1.5 2011-02-28 00:58:03 remko Exp $
#
. functions.sh

pscoast -Rd -JR4.5i -Bg30/g15 -Dc -A10000 -Ggray -P > GMT_winkel.ps
