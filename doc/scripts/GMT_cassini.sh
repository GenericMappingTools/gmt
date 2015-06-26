#!/bin/bash
#	$Id$
#
gmt pscoast -R7:30/38:30/10:30/41:30r -JC8.75/40/2.5i -Bafg -Lg9.5/38.8+c40+w60+f -Dh -Gspringgreen \
	-Sazure -Wthinnest -Ia/thinner -P --FONT_LABEL=10p > GMT_cassini.ps
