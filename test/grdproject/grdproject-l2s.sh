#!/usr/bin/env bash
#
# Test grdproject longopts translation.

m=grdproject
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -C20/10
--l2stranstest -D12.5+e/10+n
--l2stranstest -D12.5+e -D12+n/10.2+e
--l2stranstest -E300
--l2stranstest -Fc -Fi -Fp
--l2stranstest -Fe -Ff -Fk
--l2stranstest -FM -Fn -Fu
--l2stranstest -Gfile.grd=nf+d2+n-99
--l2stranstest -Gother_file.grd=nf+o6+s1.5
--l2stranstest -I
--l2stranstest -Mc -Mi -Mp
EOF

# module-specific longopts
gmt $m $l2s --center=20/10 >> $b
gmt $m $l2s --increment=12.5+exact/10+number >> $b
gmt $m $l2s --inc=12.5+exact --spacing=12+number/10.2+exact >> $b
gmt $m $l2s --dpi=300 >> $b
gmt $m $l2s --one2one_scale=c --one2one=i --one2one=p >> $b
gmt $m $l2s --one2one=e --one2one=f --one2one=k >> $b
gmt $m $l2s --one2one=M --one2one=n --one2one=u >> $b
gmt $m $l2s --outgrid=file.grd=nf+divide:2+nan:-99 >> $b
gmt $m $l2s --outgrid=other_file.grd=nf+offset:6+scale:1.5 >> $b
gmt $m $l2s --inverse >> $b
gmt $m $l2s --projected_unit=c --unit=i --unit=p >> $b

diff $a $b --strip-trailing-cr > fail
