#!/usr/bin/env bash
cat << EOF > p.txt
> -LFirst
0 0
1 0
1 1
> 2 2 -L"2nd item"
3 2
3 3
2 3
> -LCurve
3 3
4 3
3 4
> -L"My Line"
4 4
5 5
4 5
EOF
gmt begin autolegendcolorL ps
  gmt set COLOR_SET red,black,cyan,darkgreen
  gmt subplot begin 2x2 -R-1/6/-1/6 -Fs8c -Srl -Scb -Blrtb -A -T"Test auto-color line legends"
    gmt plot p.txt -R-1/6/-1/6 -W3p,auto -l"Line #"+jBR -c
    gmt plot p.txt -R-1/6/-1/6 -W3p,auto -l"Line X-%2.2d"+jBR -c
    gmt plot p.txt -R-1/6/-1/6 -W3p,auto -l"Line A,Line B,Line C,N/A"+jBR -c
    gmt plot p.txt -R-1/6/-1/6 -W3p,auto -l+jBR -c
  gmt subplot end
gmt end show
