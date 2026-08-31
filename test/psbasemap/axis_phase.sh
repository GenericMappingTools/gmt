#!/usr/bin/env bash
#
# Test that a phase shift appended to a -B interval only shifts the item it belongs to.
# See https://github.com/GenericMappingTools/gmt/issues/8993
#
# No baseline PostScript is needed: we compare the plot records from a single -B call
# with those from the two-call sequence that has always placed the items correctly.

# Keep the moveto/lineto records and the annotation strings of the plot itself.  The
# two-call reference draws the frame twice, hence the sort -u.
extract () {
	sed -n '/BeginObject/,/EndObject/p' "$1" | grep -E '^(N )?-?[0-9]+ -?[0-9]+ (M|D)|^\(.*\) (mr|bc) Z' | sort -u
}

# 1. Phase on the grid interval only: annotations and ticks must stay put
gmt psbasemap -R0/10/0/10 -JX5c -Ba5f1g1+0.5 -P > one.ps
gmt psbasemap -R0/10/0/10 -JX5c -Ba5f1 -P -K > two.ps
gmt psbasemap -R -J -Bg1+0.5 -O >> two.ps
extract one.ps > one.txt
extract two.ps > two.txt
diff one.txt two.txt > fail

# 2. Repeating the phase on every item still shifts everything alike
gmt psbasemap -R0/10/0/10 -JX5c -Ba5+0.5f1+0.5g1+0.5 -P > all.ps
gmt psbasemap -R0/10/0/10 -JX5c -Ba5+0.5f1+0.5 -P -K > allref.ps
gmt psbasemap -R -J -Bg1+0.5 -O >> allref.ps
extract all.ps > all.txt
extract allref.ps > allref.txt
diff all.txt allref.txt >> fail

# 3. The annotations of case 1 are the unshifted 0, 5 and 10
cat << EOF > annot_answer.txt
0
5
10
EOF
grep -E '^\(.*\) bc Z' one.ps | tr -d '()' | awk '{print $1}' | sort -n -u > annot.txt
diff annot.txt annot_answer.txt >> fail
