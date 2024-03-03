#!/usr/bin/env bash
#
# Test pslegend longopts translation.

m=pslegend
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -C1/2
--l2stranstest -D10/20/2/3 -Dg1/-0.5+w7
--l2stranstest -DjBR+jLT+l2
--l2stranstest -DJBR+o1/2
--l2stranstest -Dn0/0 -Dx5/5
--l2stranstest -F+c10+gred+i4p/black
--l2stranstest -F+p2p+r1p+s1/2/gray
--l2stranstest -Mh -Me
--l2stranstest -S4
--l2stranstest -T/my/secrets -T/your/secrets
EOF

# module-specific longopts
gmt $m $l2s --clearance=1/2 >> $b
gmt $m $l2s --position=10/20/2/3 --position=mapcoords:1/-0.5+width:7 >> $b
gmt $m $l2s --position=inside:BR+janchor:LT+spacing:2 >> $b
gmt $m $l2s --position=outside:BR+anchoroffset:1/2 >> $b
gmt $m $l2s --position=boxcoords:0/0 --position=plotcoords:5/5 >> $b
gmt $m $l2s --box+clearance:10+fill:red+inner:4p/black >> $b
gmt $m $l2s --box+pen:2p+radius:1p+shade:1/2/gray >> $b
gmt $m $l2s --source=hidden --source=explicit >> $b
gmt $m $l2s --scale=4 >> $b
gmt $m $l2s --hidden_file=/my/secrets --leg_file=/your/secrets >> $b

diff $a $b --strip-trailing-cr > fail
