#!/bin/sh
#	$Id: GMT_cassini.sh,v 1.4 2004-07-13 18:47:09 pwessel Exp $
#

pscoast -R7:30/38:30/10:30/41:30r -JC8.75/40/2.5i -B1g1f30m -Lf9.5/38.8/40/60 -Dh -Glightgray \
   -W0.25p -Ia/0.5p -P --LABEL_FONT_SIZE=12 > GMT_cassini.ps
