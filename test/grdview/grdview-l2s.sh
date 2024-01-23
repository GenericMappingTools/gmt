#!/usr/bin/env bash
#
# Test grdview longopts translation.

m=grdview
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -C/some/file.cpt -C/some/file.cpt
--l2stranstest -G/this/file.grd -G/that/file.grd
--l2stranstest -I/lighting/file+a
--l2stranstest -I+d -I6+m6+nsome.args
--l2stranstest -N200+ggreen
--l2stranstest -Qm+m -Qmx+m
--l2stranstest -S
--l2stranstest -T+o2p,black+s
--l2stranstest -Wc -Wm -Wf
EOF

# module-specific longopts
gmt $m $l2s --cpt=/some/file.cpt --cmap=/some/file.cpt >> $b
gmt $m $l2s --drapegrid=/this/file.grd --drape=/that/file.grd >> $b
gmt $m $l2s --illumination=/lighting/file+azimuth >> $b
gmt $m $l2s --shading+default --intensity=6+ambient:6+args:some.args >> $b
gmt $m $l2s --plane=200+color:green >> $b
gmt $m $l2s --plottype=m+monochrome --surftype=mx+mono >> $b
gmt $m $l2s --smooth >> $b
gmt $m $l2s --no_interp+outlines:2p,black+skipnans >> $b
gmt $m $l2s --pen=contour --pen=mesh --pen=facade >> $b

diff $a $b --strip-trailing-cr > fail
