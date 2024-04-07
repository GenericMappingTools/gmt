#!/usr/bin/env bash
#
# Test psimage longopts translation.

m=psimage
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -D10/20/2/3+r1200
--l2stranstest -Dg1/-0.5+w7
--l2stranstest -DjBR+jLT+n2/2
--l2stranstest -DJBR+o1/2
--l2stranstest -Dn0/0 -Dx5/5
--l2stranstest -F+c10+gred+i4p/black
--l2stranstest -F+p2p+r1p+s1/2/gray
--l2stranstest -Gred+b -Gred+b
--l2stranstest -Gred+b -Ggreen+f
--l2stranstest -Ggreen+f -Ggreen+f
--l2stranstest -Gblue+t -Gblue+t
--l2stranstest -I
--l2stranstest -M
EOF

# module-specific longopts
gmt $m $l2s --position=10/20/2/3+dpi:1200 >> $b
gmt $m $l2s --position=mapcoords:1/-0.5+width:7 >> $b
gmt $m $l2s --position=inside:BR+janchor:LT+replicate:2/2 >> $b
gmt $m $l2s --position=outside:BR+anchoroffset:1/2 >> $b
gmt $m $l2s --position=boxcoords:0/0 --position=plotcoords:5/5 >> $b
gmt $m $l2s --box+clearance:10+fill:red+inner:4p/black >> $b
gmt $m $l2s --box+pen:2p+radius:1p+shade:1/2/gray >> $b
gmt $m $l2s --bitcolor=red+bg --bit_color=red+background >> $b
gmt $m $l2s --bitcolor=red+bit_bg --bitcolor=green+fg >> $b
gmt $m $l2s --bitcolor=green+foreground --bitcolor=green+bit_fg >> $b
gmt $m $l2s --bitcolor=blue+alpha --bitcolor=blue+bit_alpha >> $b
gmt $m $l2s --invert >> $b
gmt $m $l2s --monochrome >> $b

diff $a $b --strip-trailing-cr > fail
