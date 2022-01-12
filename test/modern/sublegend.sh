#!/usr/bin/env bash
# Test that the auto-legend works in subplots
# DVC_TEST
gmt begin sublegend ps
  gmt subplot begin 2x1 -Fs7i/3i -T"Subplots with legends" -X0.7i
    gmt plot -R0/7/3/7 @Table_5_11.txt -Sc0.15i -Glightgreen -Wfaint -lApples+HLEGEND+f16p+D -c0
    gmt plot @Table_5_11.txt -W1.5p,gray -l"My Lines"+S0.2i
    gmt plot @Table_5_11.txt -St0.15i -Gorange -lOranges
    gmt plot -R0/7/3/7 @Table_5_11.txt -Sc0.15i -Glightgreen -Wfaint -lApples+HLEGEND+f16p+D+jTL -c1
    gmt plot @Table_5_11.txt -W1.5p,gray -l"My Lines"+S0.2i
    gmt plot @Table_5_11.txt -St0.15i -Gred -lRed
  gmt subplot end
gmt end show
