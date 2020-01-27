#!/usr/bin/env bash
cat << EOF >| xannots.txt
416.0	ig	Devonian
443.7	ig	Silurian
488.3	ig	Ordovician
542	ig	Cambrian
EOF
cat << EOF >| yannots.txt
0	a
1	a
2	f
2.71828	ag	e
3	f
3.1415926	ag	@~p@~
4	f
5	f
6	f
6.2831852	ag	2@~p@~
EOF

gmt begin GMT_-B_custom
	gmt basemap -R416/542/0/6.2831852 -JX-12c/6c -Bpx25f5g25+u" Ma" -Bpycyannots.txt -BWS+glightblue
	gmt basemap -R416/542/0/6.2831852 -Bsxcxannots.txt -Bsy0 -BWS \
		--MAP_ANNOT_OFFSET_SECONDARY=10p --MAP_GRID_PEN_SECONDARY=2p
gmt end show
