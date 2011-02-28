#!/bin/bash
#	$Id: GMT_equi_cyl.sh,v 1.5 2011-02-28 00:58:03 remko Exp $
#
. functions.sh

pscoast -Rg -JQ4.5i -B60f30g30 -Dc -A5000 -Gblack -P > GMT_equi_cyl.ps
