#!/bin/bash
#	$Id: GMT_mollweide.sh,v 1.5 2011-02-28 00:58:02 remko Exp $
#
. functions.sh

pscoast -Rd -JW4.5i -Bg30/g15 -Dc -A10000 -Gblack -P > GMT_mollweide.ps
