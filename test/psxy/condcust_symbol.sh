#!/bin/bash
# Test custom symbol with conditional statements.  From Kristof
# http://gmt.soest.hawaii.edu/boards/1/topics/7399

ps=condcust_symbol.ps

cat << EOF > sample_data.txt
# Just some test data
# LON  LAT  SIZE  LAT2  VALUE
118  2 1  2 10
119  2 1  2 20
120  2 1  2 30
121  2 1  2 40
122  2 1  2 50
118  1 1  1 60
119  1 1  1 70
120  1 1  1 80
121  1 1  1 90
122  1 1  1 100
118  0 1  0 110
119  0 1  0 120
120  0 1  0 130
121  0 1  0 140
122  0 1  0 150
118 -1 1 -1 160
119 -1 1 -1 170
120 -1 1 -1 10
121 -1 1 -1 20
122 -1 1 -1 30
EOF
cat << EOF > cond_test_symbol.def
N: 2 oo
if \$1 >= 0 then {
  if \$2 > 100 then {
    0 0 0.5 c -W1p,black -G-
  } elseif \$2 > 50 then {
    0 0 0.5 d -W1p,blue -G-
  } else {
    0 0 0.5 a -W1p,orange -G-
  }
  0 0 1 c -W1p,red -G-
} else {
  0 0 1 c -W1p,green -G-
}
EOF
gmt pscoast -R117/-1.5/122.5/3+r -JM15c -Bag -Di -Ggrey -Wthinnest -A250 -P -K -Xc > $ps
gmt psxy -R -J sample_data.txt -Skcond_test_symbol -O --PROJ_LENGTH_UNIT=cm >> $ps
