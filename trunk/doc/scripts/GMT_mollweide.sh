#!/bin/bash
#	$Id: GMT_mollweide.sh,v 1.7 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh

pscoast -Rd -JW4.5i -Bg30/g15 -Dc -A10000 -Gblack -P > GMT_mollweide.ps
