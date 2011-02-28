#!/bin/bash
#	$Id: GMT_cassini.sh,v 1.7 2011-02-28 00:58:03 remko Exp $
#
. functions.sh

pscoast -R7:30/38:30/10:30/41:30r -JC8.75/40/2.5i -B1g1f30m -Lf9.5/38.8/40/60 -Dh -Glightgray \
	-Wthinnest -Ia/thinner -P --LABEL_FONT_SIZE=12 > GMT_cassini.ps
