#!/usr/bin/env bash
#
# Test inset longopts translation.

m=inset
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Cw25
--l2stranstest -D10/20/2/3+r+ui
--l2stranstest -Dg1/-0.5+w7
--l2stranstest -DjBR+jLT
--l2stranstest -DJBR+o1/2
--l2stranstest -Dn0/0 -Dx5/5
--l2stranstest -F+c10+gred+i4p/black
--l2stranstest -F+p2p+r1p+s1/2/gray
--l2stranstest -N -N
EOF

# module-specific longopts
# (note 'gmt {begin,end}' bracketing as module not available in classic mode)
gmt begin
gmt $m $l2s --clearance=w25 >> $b
gmt $m $l2s --rectangle=10/20/2/3+corners+units:i >> $b
gmt $m $l2s --inset_box=mapcoords:1/-0.5+width:7 >> $b
gmt $m $l2s --position=inside:BR+anchor:LT >> $b
gmt $m $l2s --rectangle=outside:BR+offset:1/2 >> $b
gmt $m $l2s --rectangle=boxcoords:0/0 --rectangle=plotcoords:5/5 >> $b
gmt $m $l2s --inset_frame+clearance:10+fill:red+inner:4p/black >> $b
gmt $m $l2s --box+pen:2p+radius:1p+shade:1/2/gray >> $b
gmt $m $l2s --noclip --no_clip >> $b
gmt end

diff $a $b --strip-trailing-cr > fail
