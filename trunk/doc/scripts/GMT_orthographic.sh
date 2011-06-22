#!/bin/bash
#	$Id: GMT_orthographic.sh,v 1.8 2011-06-22 00:35:06 remko Exp $
#
. ./functions.sh

pscoast -Rg -JG-75/41/4.5i -B15g -Dc -A5000 -Gpink -Sthistle -P > GMT_orthographic.ps
