#!/bin/bash
#	$Id: GMT_cassini.sh,v 1.12 2011-06-23 01:17:06 remko Exp $
#
. ./functions.sh

pscoast -R7:30/38:30/10:30/41:30r -JC8.75/40/2.5i -B1g1f30m -Lf9.5/38.8/40/60 -Dh -Gspringgreen \
	-Sazure -Wthinnest -Ia/thinner -P --FONT_LABEL=12p > GMT_cassini.ps
