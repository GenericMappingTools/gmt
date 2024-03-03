#!/usr/bin/env bash
#
# Test sph2grd longopts translation.

m=sph2grd
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Dg -Dn
--l2stranstest -E
--l2stranstest -F-/-/50/75 -Fk1/2/3/4
--l2stranstest -Fk1/2/3/4
--l2stranstest -G/some/file.grd=nf+d2+n-99
--l2stranstest -G/other/file.grd=nf+o6+s1.5
--l2stranstest -I5+e/10 -I2 -I1/2+n
--l2stranstest -Ng -Nm -Ns
--l2stranstest -Q
EOF

# module-specific longopts
gmt $m $l2s --derive=gravity --derive=geoid >> $b
gmt $m $l2s --ellipsoid >> $b
gmt $m $l2s --filter=-/-/50/75 --filter=km:1/2/3/4 >> $b
gmt $m $l2s --filter=kilometers:1/2/3/4 >> $b
gmt $m $l2s --outgrid=/some/file.grd=nf+divide:2+nan:-99 >> $b
gmt $m $l2s --outgrid=/other/file.grd=nf+offset:6+scale:1.5 >> $b
gmt $m $l2s --increment=5+exact/10 --spacing=2 --inc=1/2+number >> $b
gmt $m $l2s --normalize=geodesy --normalize=math --normalize=schmidt >> $b
gmt $m $l2s --phaseconv >> $b

diff $a $b --strip-trailing-cr > fail
