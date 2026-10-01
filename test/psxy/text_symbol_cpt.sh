#!/usr/bin/env bash
# Test that the -Sl text symbol is filled with the color from -C, with and without -W,
# for psxy and psxyz; it used to be drawn black (or hollow with -W).
gmt makecpt -Cred,blue -T0.5/2.5/1 > rb.cpt
echo 0 0 1 | gmt psxy -R-5/5/-5/5 -JX10c -Sl2c+tTEXT -Crb.cpt > fill.ps
echo 0 0 1 | gmt psxy -R-5/5/-5/5 -JX10c -Sl2c+tTEXT -Crb.cpt -W1p > pen.ps
echo 0 0 0 1 | gmt psxyz -R-5/5/-5/5/0/1 -JX10c -JZ2c -Sl2c+tTEXT -Crb.cpt > fill3d.ps
for f in fill pen fill3d; do
	awk '/^\{1 0 0 C\} FS$/ {fs = 1} /^FQ$/ {fs = 0} /\(TEXT\) mc false charpath fs/ {ok = fs} END {exit !ok}' $f.ps || echo "$f: text not filled with the CPT color" >> fail
done
