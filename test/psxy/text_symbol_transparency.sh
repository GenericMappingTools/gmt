#!/usr/bin/env bash
# Test that the fill transparency of the -Sl text symbol stays local to the text
# (issue 9234); otherwise it leaks into later plot layers, e.g., following subplot panels.
# The text must be painted via fs and no global PSL_transp may be set.
echo 0 0 | gmt psxy -R-5/5/-5/5 -JX10c -Sl2c+tTEXT -Gred@50 > psxy.ps
echo 0 0 0 | gmt psxyz -R-5/5/-5/5/-5/5 -JX10c -JZ10c -p150/30 -Sl2c+tTEXT -Gred@50 > psxyz.ps
for f in psxy psxyz; do
	grep -q "(TEXT) mc false charpath fs N" $f.ps || echo "$f: text not painted via fs" >> fail
	if grep -Eq "[0-9.] /[A-Za-z]+ PSL_transp$" $f.ps; then echo "$f: global transparency set" >> fail; fi
done
