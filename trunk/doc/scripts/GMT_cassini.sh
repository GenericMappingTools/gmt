#!/bin/sh
#	$Id: GMT_cassini.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#

gmtset LABEL_FONT_SIZE 12
pscoast -R7:30/38:30/10:30/41:30r -JC8.75/40/2.5i -B1g1f30m -Lf9.5/38.8/40/60 -Dh -G200 -W0.25p \
   -Ia/0.5p -P > GMT_cassini.ps
