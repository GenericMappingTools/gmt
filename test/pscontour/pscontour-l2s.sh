#!/usr/bin/env bash
#
# Test pscontour longopts translation.

m=pscontour
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -An -A1+an+c1/2+d
--l2stranstest -A5+e+fHelvetica+gred+i
--l2stranstest -A2+jRT+n0.1/0.2+o
--l2stranstest -A1+p1p,blue+r2+t/some/file
--l2stranstest -A2+uz+v+w3
--l2stranstest -A3+xf,l+=MeFirst
--l2stranstest -C/some/nice.cpt -C/contour/file
--l2stranstest -C100
--l2stranstest -D
--l2stranstest -E/index/file+b
--l2stranstest -Gd5 -GD50
--l2stranstest -Gf/some/file -GlLB
--l2stranstest -GLRT
--l2stranstest -Gn20 -GN10
--l2stranstest -Gx/this/sfile -GX/that/cfile
--l2stranstest -I
--l2stranstest -L2p -L3p,black
--l2stranstest -N -N
--l2stranstest -Q25c -Q100+z -Q10
--l2stranstest -Sp -St
--l2stranstest -Th+a -T"l+d12/5+lsome lbls"
--l2stranstest -Wa2p+cl -Wc1p
EOF

# module-specific longopts
gmt $m $l2s --annotation=none --annot=1+angle:n+clearance:1/2+debug >> $b
gmt $m $l2s --annot=5+delay+font:Helvetica+fill:red+nolines >> $b
gmt $m $l2s --annot=2+justify:RT+nudge:0.1/0.2+rounded >> $b
gmt $m $l2s --annot=1+pen:1p,blue+minradius:2+labelfile:/some/file >> $b
gmt $m $l2s --annot=2+unit:z+curved+npoints:3 >> $b
gmt $m $l2s --annot=3+firstlast:f,l+prefix:MeFirst >> $b
gmt $m $l2s --contours=/some/nice.cpt --levels=/contour/file >> $b
gmt $m $l2s --contours=100 >> $b
gmt $m $l2s --dump >> $b
gmt $m $l2s --index=/index/file+binary >> $b
gmt $m $l2s --labels=plotdist:5 --label_placement=mapdist:50 >> $b
gmt $m $l2s --labels=locfile:/some/file --labels=segments:LB >> $b
gmt $m $l2s --labels=circles:RT >> $b
gmt $m $l2s --labels=nlabels:20 --labels=linestart:10 >> $b
gmt $m $l2s --labels=segmentfile:/this/sfile --labels=circlefile:/that/cfile >> $b
gmt $m $l2s --colorize >> $b
gmt $m $l2s --mesh=2p --triangular_mesh_pen=3p,black >> $b
gmt $m $l2s --noclip --no_clip >> $b
gmt $m $l2s --cut=25c --minpoints=100+nonzero --minlength=10 >> $b
gmt $m $l2s --skip=points --skip=triangles >> $b
gmt $m $l2s --ticks=high+all --ticks=low+gap:12/5+labels:"some lbls" >> $b
gmt $m $l2s --pen=annot:2p+colors:l --pen=regular:1p >> $b

diff $a $b --strip-trailing-cr > fail
