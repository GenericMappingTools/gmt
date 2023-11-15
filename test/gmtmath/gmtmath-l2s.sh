#!/usr/bin/env bash
#
# Test gmtmath longopts translation.

m=gmtmath
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A/some/file+e -A/some/file+r
--l2stranstest -A/some/file+s -A/some/file+w
--l2stranstest -C -C1,3-5,7 -Cx -Cy
--l2stranstest -C0 -Ca -Cr
--l2stranstest -E1e-8
--l2stranstest -I -I
--l2stranstest -N8/1
--l2stranstest -Qc -Qc
--l2stranstest -Qi -Qp -Qn
--l2stranstest -S -Sf -Sl
--l2stranstest -T10/20/0.5+b -T10/20/0.5+l
--l2stranstest -T10/20/0.5+i -T10/20/0.5+n
--l2stranstest -T/some/file
EOF

# module-specific longopts
gmt $m $l2s --init=/some/file+evaluate --matrix=/some/file+no_left >> $b
gmt $m $l2s --init=/some/file+sigma --init=/some/file+weights >> $b
gmt $m $l2s --columns --columns=1,3-5,7 --columns=x --columns=y >> $b
gmt $m $l2s --columns=0  --columns=a --columns=r >> $b
gmt $m $l2s --eigen=1e-8 >> $b
gmt $m $l2s --invert --reverse >> $b
gmt $m $l2s --ncolumns=8/1 >> $b
gmt $m $l2s --quick=cm --quick=centimeter >> $b
gmt $m $l2s --quick=inch --quick=point --quick=none >> $b
gmt $m $l2s --output --output=first --output=last >> $b
gmt $m $l2s --range=10/20/0.5+log2 --range=10/20/0.5+log10 >> $b
gmt $m $l2s --series=10/20/0.5+inverse --series=10/20/0.5+number >> $b
gmt $m $l2s --series=/some/file >> $b

diff $a $b --strip-trailing-cr > fail
