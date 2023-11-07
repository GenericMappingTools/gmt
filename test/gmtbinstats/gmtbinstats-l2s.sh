#!/usr/bin/env bash
#
# Test gmtbinstats longopts translation.

m=gmtbinstats
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Ca -Cd -Cg
--l2stranstest -Ci -Cl -CL
--l2stranstest -Cm -Cn -Co
--l2stranstest -Cp -Cq75 -Cr
--l2stranstest -Cs -Cu -CU
--l2stranstest -Cz
--l2stranstest -E
--l2stranstest -G/somefile=nf+d10+n-999+o6
--l2stranstest -G/otherfile+oa
--l2stranstest -G/otherfile+s100+c
--l2stranstest -I12+e/15+n
--l2stranstest -N
--l2stranstest -S100m -S10m
--l2stranstest -Th -Tr
--l2stranstest -W -W+s -W+s
EOF

# module-specific longopts
gmt $m $l2s --statistic=mean --statistic=mad --statistic=full >> $b
gmt $m $l2s --statistic=interquartile --statistic=min --statistic=minpos >> $b
gmt $m $l2s --statistic=median --statistic=number --statistic=lms >> $b
gmt $m $l2s --statistic=mode --statistic=quantile:75 --statistic=rms >> $b
gmt $m $l2s --statistic=stddev --statistic=max --statistic=maxneg >> $b
gmt $m $l2s --statistic=sum >> $b
gmt $m $l2s --empty >> $b
gmt $m $l2s --outgrid=/somefile=nf+divide:10+nan:-999+offset:6 >> $b
gmt $m $l2s --outgrid=/otherfile+offset:a >> $b
gmt $m $l2s --outgrid=/otherfile+scale:100+gdal >> $b
gmt $m $l2s --increment=12+exact/15+number >> $b
gmt $m $l2s --normalize >> $b
gmt $m $l2s --search_radius=100m --radius=10m >> $b
gmt $m $l2s --tiling=hexagonal --tiling=rectangular >> $b
gmt $m $l2s --weight --weight+sigma --weight+uncertainty >> $b

diff $a $b --strip-trailing-cr > fail
