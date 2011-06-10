#!/bin/bash
#	$Id: GMT_general_cyl.sh,v 1.8 2011-06-10 23:29:27 guru Exp $
#
. ./functions.sh

pscoast -R-145/215/-90/90 -JY35/30/4.5i -B45g45 -Dc -A10000 -Sdodgerblue -Wthinnest -P > \
	GMT_general_cyl.ps
