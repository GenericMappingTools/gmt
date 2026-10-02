#!/usr/bin/env bash
#
# Test that a ?-width in -J inside an inset resolves to the inset size for every -D form.
# See https://github.com/GenericMappingTools/gmt/issues/9224
#
# No baseline PostScript is needed: we compare the inset map dimensions reported by
# mapproject -W with the inset sizes computed from the -D arguments.
cat << EOF > answer.txt
1.625	1.87095
1.625	1.87095
1.45976	1.45976
0.729881	0.729881
2	1.5
EOF
gmt begin
gmt figure inset_size ps
	gmt basemap -R0/40/20/60 -JM6.5i -B
	for D in 5/15/25/35 5/25/15/35+r -1000/0/3000/4000+uk jTR+w500k jTR+w2i/1.5i; do
		gmt inset begin -D$D
			gmt basemap -R0/1/0/1 -JX?/? -B
			gmt mapproject -W -Di >> result.txt
		gmt inset end
	done
gmt end
diff result.txt answer.txt --strip-trailing-cr > fail
