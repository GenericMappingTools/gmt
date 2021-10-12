#!/usr/bin/env bash
# Test to show we fixed https://github.com/GenericMappingTools/gmt/issues/5839
#
cat << EOF >| annots.txt
0 a 0
3.1415926 a @~p@~
10 a 10
15 a 15
20 a 20
EOF

gmt begin vertcustannot ps
	gmt basemap -R-10/10/0/10 -B
	gmt makecpt -Cjet -T0/20
	gmt colorbar -DJMR -Bpxcannots.txt
	gmt colorbar -DJBC -Bpxcannots.txt
	gmt colorbar -DJTC -Bpxcannots.txt
	gmt colorbar -DJML -Bpxcannots.txt
gmt end show
