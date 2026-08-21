#!/usr/bin/env bash
#
# Check psmeca -Sm, -Sz and -Sd
#
# The data files and original script are provided by Carl Tape,
# available from https://github.com/carltape/compearth, licensed under MIT license.
#
# Also see https://github.com/GenericMappingTools/gmt/pull/2135
#
ps=meca_-Smdz.ps

gmt gmtset PS_MEDIA Custom_12ix9.5i
gmt psbasemap -JH0/2.8i -R-30/30/-90/90 -Bxa10f5g10 -Bya10f5g10 -Bwesn -K -P > $ps
gmt psmeca fullmt_ipts1_iref1 -J -R -Sm0.45i/8p -L0.5p,0/0/0 -G255/0/0 -N -K -O >> $ps

gmt psbasemap -JH0/2.8i -R-30/30/-90/90 -Bxa10f5g10 -Bya10f5g10 -Bwesn -K -O -X3.5i >> $ps
gmt psmeca fullmt_ipts1_iref1 -J -R -Sz0.45i/8p -L0.5p,0/0/0 -G255/0/0 -N -K -O >> $ps

gmt psbasemap -JH0/2.8i -R-30/30/-90/90 -Bxa10f5g10 -Bya10f5g10 -Bwesn -K -O -X3.5i >> $ps
gmt psmeca fullmt_ipts1_iref1 -J -R -Sd0.45i/8p -L0.5p,0/0/0 -G255/0/0 -N -O >> $ps
