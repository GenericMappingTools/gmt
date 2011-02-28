#!/bin/bash
#	$Id: GMT_orthographic.sh,v 1.5 2011-02-28 00:58:03 remko Exp $
#
. functions.sh

pscoast -Rg -JG-75/41/4.5i -B15g15 -Dc -A5000 -Gblack -P > GMT_orthographic.ps
