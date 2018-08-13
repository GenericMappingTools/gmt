#!/bin/bash
# Test custom symbol with conditional statements accessing lon, lat or x,y

ps=condcust_symbol_xy.ps
cat << EOF > sample_data.txt
1	1
-1	1
-1	-1
1	-1
EOF
cat << EOF > xy.def
if \$x >= 0 then {
  if \$y > 0 then {
    0 0 1.0 c -W3p,black -G-
  } else {
    0 0 1.0 c -W3p,orange -G-
  }
} else {
  if \$y > 0 then {
    0 0 1.0 c -W3p,red -G-
  } else {
    0 0 1.0 c -W3p,green -G-
  }
}
EOF
# First plot as Cartesian
gmt psxy -R-2/2/-2/2 -JX4i -P -Bafg1 -BWSne -Skxy/1i sample_data.txt -Xc -K > $ps
# Second plot as Geographic
gmt psxy -R -JM4i -O -Bafg1 -BWSne -Skxy/1i sample_data.txt -Y4.75i >> $ps
