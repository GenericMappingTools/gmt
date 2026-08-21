#!/usr/bin/env bash
#
# Test grdclip longopts translation.

m=grdclip
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -G/somefile=nf+d10+n-999+o6
--l2stranstest -G/otherfile+oa
--l2stranstest -G/otherfile+s100
--l2stranstest -Sa100/200+e -Sb50/0+e
--l2stranstest -Si100/200/150
--l2stranstest -Sr5/0 -Sr6/0
EOF

# module-specific longopts
gmt $m $l2s --outgrid=/somefile=nf+divide:10+nan:-999+offset:6 >> $b
gmt $m $l2s --outgrid=/otherfile+offset:a >> $b
gmt $m $l2s --outgrid=/otherfile+scale:100 >> $b
gmt $m $l2s --set=above:100/200+equal --set=below:50/0+equal >> $b
gmt $m $l2s --set=between:100/200/150 >> $b
gmt $m $l2s --set=replace:5/0 --set=new:6/0 >> $b

diff $a $b --strip-trailing-cr > fail
