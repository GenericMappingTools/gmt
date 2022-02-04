#!/usr/bin/env bash
# Test that the auto-legend works in a map inset
gmt begin insetlegend ps
  gmt psbasemap -R0/7/3/7 -Jx1i -B0 -B+t"Inset with plot and legend" -Xc
  gmt inset begin -DjTR+w3.8i/2.5i+o0.1i -M1c -F+p1p,red
    gmt plot -R0/7/3/7 -B @Table_5_11.txt -Sc0.15i -Glightgreen -Wfaint -lApples+HLEGEND+f16p+D+jTL+s0.5+glightblue+p2p+o0.3c
    gmt plot @Table_5_11.txt -W1.5p,gray -l"My Lines"
    gmt plot @Table_5_11.txt -St0.15i -Gorange -lOranges
  gmt inset end
gmt end show
