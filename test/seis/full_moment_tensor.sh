#!/usr/bin/env bash
#
# Check full moment tensor, especially with big isotropic components.
#
# The data files and original script are provided by Carl Tape,
# available from https://github.com/carltape/compearth, licensed under MIT license.
#
# Also see https://github.com/GenericMappingTools/gmt/issues/661.
#
ps=full_moment_tensor.ps
gmt set PS_MEDIA 21.5ix9.5i MAP_TICK_LENGTH 0.0c

gmt psxy -JH0/2.8i -R-30/30/-90/90 -T -K -P -X0.5i -Y0.5i > $ps
for dfile in fullmt_ipts1_iref1 \
			 fullmt_ipts1_iref2 \
			 fullmt_ipts1_iref3 \
			 fullmt_ipts1_iref4 \
			 fullmt_ipts1_iref5 \
			 fullmt_ipts2_iref3; do
	gmt psbasemap -J -R -Bxa10f5g10 -Bya10f5g10 -Bwesn+g200 -K -O >> $ps
	gmt psmeca ${dfile} -J -R -Sm0.45i/8p -L0.5p,0/0/0 -G255/0/0 -N -K -O >> $ps
	gmt psxy -J -R -T -K -O -X3.5i >> $ps
done

gmt psxy -R -J -O -T >> $ps
gmt psconvert -A -P -Tf $ps
rm gmt.conf gmt.history
