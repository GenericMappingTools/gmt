#!/bin/bash
#	$Id: GMT_equi_cyl.sh,v 1.7 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh

pscoast -Rg -JQ4.5i -B60f30g30 -Dc -A5000 -Gblack -P > GMT_equi_cyl.ps
