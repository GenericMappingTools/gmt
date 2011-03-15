#!/bin/bash
#	$Id: GMT_orthographic.sh,v 1.6 2011-03-15 02:06:29 guru Exp $
#
. functions.sh

pscoast -Rg -JG-75/41/4.5i -B15g15 -Dc -A5000 -Gpink -Sthistle -P > GMT_orthographic.ps
