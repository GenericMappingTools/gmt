#!/bin/bash
#	$Id$
#
gmt pscoast -R130/150/35/50 -JM6i -B5 -P -Ggray -K > GMT_tut_9.ps
cp ../tutorial/quakes.cpt .
gmt psxy -R -J -O ../tutorial/quakes.ngdc -Wfaint -i4,3,5,6s0.1 -h3 -Scc -Cquakes.cpt >> GMT_tut_9.ps
