#!/usr/bin/env bash
#
# Check that meca honors the z-range of the CPT given in -C.  A regular CPT file
# must be used as is (the beachball colors then only depend on the depth of the
# event, not on the depth range of the events), while a master CPT name must be
# stretched to the depth range of the events since it only has a 0-1 range.
#
# See https://github.com/GenericMappingTools/gmt/issues/9176 (regular CPT) and
# https://github.com/GenericMappingTools/gmt/issues/8966 (master CPT).

# Extract the fill colors set for the beachballs, in plot order
fills () {
	grep -o '{[0-9.]* [0-9.]* [0-9.]* C} FS' $1 | sed -e 's/[{}]//g' -e 's/ C FS//'
}

cat << EOF > two.txt
239.0 34.0 10 180 18 -88 5
239.5 34.5 40 180 18 -88 5
EOF
cat << EOF > three.txt
239.0 34.0 10 180 18 -88 5
239.5 34.5 40 180 18 -88 5
239.8 34.8 90 180 18 -88 5
EOF
cat << EOF > pts.txt
239.0 34.0 10
239.5 34.5 40
EOF

gmt makecpt -Cbatlow -T0/100 > depth.cpt

# 1. With a CPT file the colors of the first two events must not depend on
#    whether or not a deeper third event is present in the input file.
gmt psmeca two.txt   -R238.5/240/33.5/35 -JM5c -Sa1c -Cdepth.cpt -P > file2.ps
gmt psmeca three.txt -R238.5/240/33.5/35 -JM5c -Sa1c -Cdepth.cpt -P > file3.ps
fills file2.ps > a.txt
fills file3.ps | head -n 2 > b.txt

# 2. Those colors must be the ones psxy assigns to the same depths via -C
gmt psxy pts.txt -R238.5/240/33.5/35 -JM5c -Sc0.5c -Cdepth.cpt -P > xy.ps
fills xy.ps > c.txt

# 3. A master CPT name has no useful range of its own, so it is stretched to the
#    depth range of the events; the extreme events then get the extreme colors.
gmt psmeca two.txt   -R238.5/240/33.5/35 -JM5c -Sa1c -Cbatlow -P > master2.ps
gmt psmeca three.txt -R238.5/240/33.5/35 -JM5c -Sa1c -Cbatlow -P > master3.ps
fills master2.ps > d.txt
fills master3.ps | sed -n '1p;3p' > e.txt

diff a.txt b.txt --strip-trailing-cr > fail
diff a.txt c.txt --strip-trailing-cr >> fail
diff d.txt e.txt --strip-trailing-cr >> fail
