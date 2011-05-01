#!/bin/bash
#	$Id: GMT_orthographic.sh,v 1.7 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh

pscoast -Rg -JG-75/41/4.5i -B15g15 -Dc -A5000 -Gpink -Sthistle -P > GMT_orthographic.ps
