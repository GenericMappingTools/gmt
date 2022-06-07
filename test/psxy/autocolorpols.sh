#!/usr/bin/env bash
# Test auto-color sequencing for fill via CPT or color list

cat << EOF > p.txt
>
0 0
1 0
1 1
0 1
>
1 1
2 1
2 2
1 2
>
2 2
3 2
3 3
2 3
>
3 3
4 3
4 4
3 4
>
4 4
5 4
5 5
4 5
EOF
gmt begin autocolorpols ps
	gmt subplot begin 2x1 -Fs10c -R-1/6/-1/6 -Sc -Sr -Blrbt
		gmt plot p.txt -Gauto -c
		gmt plot p.txt -Gauto@50 -c --COLOR_SET=red,green,blue
	gmt subplot end
gmt end show
