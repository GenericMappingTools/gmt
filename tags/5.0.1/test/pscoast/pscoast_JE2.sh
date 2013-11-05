#!/bin/bash
#
#	$Id$
# Make sure when fixed it works for all resolutions -D?

. functions.sh
header "Test pscoast for JE plots of the Pacific"

pscoast -Dl -R72.205/-47.346/310.656/30.368r -JE179.300/16.640/3i -A1000 -W0.25p -N1 -B0 -K -Y7i -P > $ps
pscoast -Dl -R -J -A1000 -G220 -W0.25p -N1 -B0 -O -K -X3.5i >> $ps
pscoast -Dl -R -J -A1000 -Slightblue -G220 -W0.25p -N1 -B0 -O -K -X-3.5 -Y-3.25i >> $ps
pscoast -Dl -R -J -A1000 -Slightblue -W0.25p -N1 -O -K -B0 -X3.5i >> $ps
pscoast -Dl -R99.811/-47.587/323.501/31.558r -JE195.470/19.970/3i -A1000 -Slightblue -W0.25p -N1 -B0 -O -K -X-3.5i -Y-3.25i >> $ps
pscoast -Dl -R -J -A1000 -Slightblue -G220 -W0.25p -N1 -B0 -O -K -X3.5i >> $ps
psxy -R -J -O -T >> $ps

pscmp
