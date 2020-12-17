#!/usr/bin/env bash
# Test auto pen color in panels and make sure ID is reset for each panel
cat << EOF > lines.txt
>
1 1
9 3
>
1 2
9 4
>
1 3
9 5
>
1 4
9 6
>
1 5
9 7
EOF
gmt begin autocolorline ps
  gmt set MAP_FRAME_TYPE plain COLOR_SET=red,green,blue
  gmt subplot begin 3x3 -Fs5c/7c -A1 -M6p -R0/10/0/8 -BWSen
    gmt plot lines.txt -W1p,auto -c
    gmt plot lines.txt -W1p,auto -c
    gmt plot lines.txt -W1p,auto -c
    
    gmt plot lines.txt -W1p,auto -c
    gmt plot lines.txt -W1p,auto -c
    gmt plot lines.txt -W1p,auto -c
    
    gmt plot lines.txt -W1p,auto -c
    gmt plot lines.txt -W1p,auto -c
    gmt plot lines.txt -W1p,auto -c
  gmt subplot end
gmt end show
