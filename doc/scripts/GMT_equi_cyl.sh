#!/bin/bash
#	$Id: GMT_equi_cyl.sh,v 1.6 2011-03-15 02:06:29 guru Exp $
#
. functions.sh

pscoast -Rg -JQ4.5i -B60f30g30 -Dc -A5000 -Gblack -P > GMT_equi_cyl.ps
