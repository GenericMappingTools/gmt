#!/usr/bin/env bash
cat << EOF > p.txt
> -LFirst
0 0
1 0
1 1
0 1
> 2 2 -L"2nd item"
2 2
3 2
3 3
2 3
> -LPoland
3 3
4 3
4 4
3 4
> -L"My Area"
4 4
5 4
5 5
4 5
EOF
gmt begin autolegendcolor ps
  gmt subplot begin 2x2 -R-1/6/-1/6 -Fs8c -Srl -Scb -Blrtb -A -T"Test auto-color polygon legends"
    gmt plot p.txt -R-1/6/-1/6 -Gauto@50 -l"Poly #"+jBR -c
    gmt plot p.txt -R-1/6/-1/6 -Gauto@50 -l"Poly X-%2.2d"+jBR -c
    gmt plot p.txt -R-1/6/-1/6 -Gauto@50 -l"Poly A,Poly B,Poly C,N/A"+jBR -c
    gmt plot p.txt -R-1/6/-1/6 -Gauto@50 -l+jBR -c
  gmt subplot end
gmt end show
