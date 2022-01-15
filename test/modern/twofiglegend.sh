#!/usr/bin/env bash
# Test that the auto-legend works in a map inset
# DVC_TEST
gmt begin
  gmt figure onefiglegend ps
    gmt plot -R0/7/3/7 -Jx1i -B @Table_5_11.txt -Sc0.15i -Glightgreen -Wfaint -lApples+HLEGEND+f16p+D
    gmt plot @Table_5_11.txt -W1.5p,gray -l"My Lines"
    gmt plot @Table_5_11.txt -St0.15i -Gorange -lOranges
  gmt figure twofiglegend ps
    gmt plot -R0/7/3/7 -Jx1i -B @Table_5_11.txt -Sc0.15i -Glightgreen -Wfaint -lApples+HLEGEND+f16p+D+jTL
    gmt plot @Table_5_11.txt -W1.5p,gray -l"My Lines"
    gmt plot @Table_5_11.txt -St0.15i -Gred -lRed
gmt end show
