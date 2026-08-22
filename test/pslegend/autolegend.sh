#!/usr/bin/env bash
gmt begin autolegend ps
  gmt plot -R0/7.2/3/7.2 -Jx1i -B @Table_5_11.txt -Sc0.15i -Glightgreen -Wfaint -lApples+H"LEGEND"+f16p+D
  gmt plot @Table_5_11.txt -W1.5p,gray -l"My Lines"
  gmt plot @Table_5_11.txt -St0.15i -Gorange -lOranges
  echo "L 9p R 2020" | gmt legend -DjTR+w1.5i+o0.1i -F+p1p+gwhite -M
gmt end show
