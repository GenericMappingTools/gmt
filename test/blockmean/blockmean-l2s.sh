#!/usr/bin/env bash
#
# Test blockmean longopts translation.

m=blockmean
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Azslhw
--l2stranstest -Ahs
--l2stranstest -C
--l2stranstest -E+p -E+P
--l2stranstest -G/some/file.grd
--l2stranstest -Sm -Sn -Ss -Sw
--l2stranstest -Wi -Wo+s
EOF

# module-specific longopts
gmt $m $l2s --fields=mean,stddev,low,high,weight >> $b
gmt $m $l2s --fields=high,stddev >> $b
gmt $m $l2s --center >> $b
gmt $m $l2s --extend+weighted --extend+simple >> $b
gmt $m $l2s --outgrid=/some/file.grd >> $b
gmt $m $l2s --statistic=mean --statistic=count --statistic=sum --statistic=weight >> $b
gmt $m $l2s --weights=in --weights=out+sigma >> $b

diff $a $b --strip-trailing-cr > fail
