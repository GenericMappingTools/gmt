#!/bin/bash
gmt begin autolegend ps
  gmt plot -R0/7.2/3/7.2 -Jx1i -B @Table_5_11.txt -Sc0.15i -Glightgreen -Wfaint -lApples+h"LEGEND"+f16p+d
  gmt plot @Table_5_11.txt -W1.5p,gray -l"My Lines"
  gmt plot @Table_5_11.txt -St0.15i -Gorange -lOranges
  gmt legend -DjTR+w1.15i+o0.1i -F+p1p
gmt end show
