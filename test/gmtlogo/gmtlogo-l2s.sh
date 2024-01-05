#!/usr/bin/env bash
#
# Test gmtlogo longopts translation.

m=gmtlogo
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Dg10/20+w5 -Dg1/2+h2
--l2stranstest -DjTR+jBL -DJTR
--l2stranstest -Dn0.5/0.4+o1/2
--l2stranstest -Dx3/4+jBL
--l2stranstest -F+c0.1 -F+gred -F+i1
--l2stranstest -F+p2p -F+r0.5 -F+s1/1/green
--l2stranstest -Sl -Sn -Su
EOF

# module-specific longopts
gmt $m $l2s --position=user:10/20+width:5 --position=map:1/2+height:2 >> $b
gmt $m $l2s --position=justify:TR+justify:BL --position=mirror:TR >> $b
gmt $m $l2s --position=normalize:0.5/0.4+offset:1/2 >> $b
gmt $m $l2s --position=plot:3/4+justify:BL >> $b
gmt $m $l2s --border+clearance:0.1 --box+fill:red --box+inner:1 >> $b
gmt $m $l2s --border+pen:2p --border+radius:0.5 --box+shade:1/1/green >> $b
gmt $m $l2s --label=standard --style=none --style=url >> $b

diff $a $b --strip-trailing-cr > fail
