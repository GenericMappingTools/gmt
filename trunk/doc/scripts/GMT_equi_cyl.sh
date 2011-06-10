#!/bin/bash
#	$Id: GMT_equi_cyl.sh,v 1.8 2011-06-10 23:29:27 guru Exp $
#
. ./functions.sh

pscoast -Rg -JQ4.5i -B60f30g30 -Dc -A5000 -Gtan4 -Slightcyan -P > GMT_equi_cyl.ps
