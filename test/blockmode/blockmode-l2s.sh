#!/usr/bin/env bash
#
# Test blockmode longopts translation.

m=blockmode
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Azslhw
--l2stranstest -C
--l2stranstest -D+a -D40+c -D+h -D+l
--l2stranstest -E -Er+l -Es+h
--l2stranstest -G/some/file.grd
--l2stranstest -Q
--l2stranstest -Wi -Wo+s
EOF

# module-specific longopts
gmt $m $l2s --fields=zslhw >> $b
gmt $m $l2s --center >> $b
gmt $m $l2s --histogram+average --histogram=40+center --histogram+high --histogram+low >> $b
gmt $m $l2s --extend --extend=record+lower --extend=source+higher >> $b
gmt $m $l2s --outgrid=/some/file.grd >> $b
gmt $m $l2s --quick >> $b
gmt $m $l2s --weights=in --weights=out+sigma >> $b

diff $a $b --strip-trailing-cr > fail
