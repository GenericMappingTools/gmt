#!/bin/bash
#
#       $Id: track.sh,v 1.1 2011-06-18 04:07:36 guru Exp $

. ../functions.sh
header "Testing mgd77track for mapping"

ps=track.ps

pscoast -R200/202/18.5/25 -JM3i -P -B1f30mWSne -K -Gbrown -Dh > $ps
mgd77track 01010221 -R -J -O >> $ps

pscmp
