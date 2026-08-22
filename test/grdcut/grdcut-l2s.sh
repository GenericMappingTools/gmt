#!/usr/bin/env bash
#
# Test grdcut longopts translation.

m=grdcut
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -D+t
--l2stranstest -Ex50 -Ey10
--l2stranstest -F+i+c
--l2stranstest -Gfile.grd=nf+d2+n-99
--l2stranstest -Gother_file.grd=nf+o6+s1.5
--l2stranstest -N-999
--l2stranstest -S100/20/5+n
--l2stranstest -Z1000/2000+n
--l2stranstest -Z1000/2000+N
--l2stranstest -Z1000/2000+r
EOF

# module-specific longopts
gmt $m $l2s --dryrun+trailer >> $b
gmt $m $l2s --extract=x:50 --extract=y:10 >> $b
gmt $m $l2s --clip+invert+crop >> $b
gmt $m $l2s --outgrid=file.grd=nf+divide:2+nan:-99 >> $b
gmt $m $l2s --outgrid=other_file.grd=nf+offset:6+scale:1.5 >> $b
gmt $m $l2s --extend=-999 >> $b
gmt $m $l2s --circ_subregion=100/20/5+set_nan >> $b
gmt $m $l2s --zrange=1000/2000+exclude_nan >> $b
gmt $m $l2s --zrange=1000/2000+include_nan >> $b
gmt $m $l2s --zrange=1000/2000+strip_nan_rowcols >> $b

diff $a $b --strip-trailing-cr > fail
