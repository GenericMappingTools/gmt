#!/usr/bin/env bash
# Test gmt pstext paragraph with outline vs outline/fill
# Making sure issue # 893 is dealt with.
ps=caption.ps

cat << EOF > tmp
This is an unmarked header record not starting with #
> 0 -0.5 13p 3i j
@%5%Figure 1.@%% This illustration shows nothing useful, but it still needs
a figure caption. Highlighted in @;255/0/0;red@;; you can see the locations
of cities where it is @_impossible@_ to get any good Thai food; these are to be avoided.
However, the bottom caption copy is outlined while the top caption is white and outlined.
EOF
gmt pstext tmp -R-1/4/0/6 -Jx1i -B0 -h1 -M -N -F+f12,Times-Roman+jLT -W1 -K -P -Y3i -Xc > $ps
gmt pstext tmp -R -J -h1 -M -N -F+f12,Times-Roman+jLT -W1 -Gwhite -O -Y3i >> $ps
