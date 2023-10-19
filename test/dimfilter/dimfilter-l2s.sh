#!/usr/bin/env bash
#
# Test dimfilter longopts translation.

m=dimfilter
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -D -D0
--l2stranstest -Fb25 -Fc100+l
--l2stranstest -Fg50+u -Fm20
--l2stranstest -Fp75+l
--l2stranstest -G/somefile=nf+d10+n-999+o6
--l2stranstest -G/otherfile+oa
--l2stranstest -G/otherfile+s100
--l2stranstest -I12+e/15+n
--l2stranstest -L
--l2stranstest -Nl -Nu
--l2stranstest -Na -Nm
--l2stranstest -Np+l -Np+u
--l2stranstest -Q
--l2stranstest -T -T
EOF

# module-specific longopts
gmt $m $l2s --distance --distance=0 >> $b
gmt $m $l2s --filter=boxcar:25 --filter=cosarch:100+lower >> $b
gmt $m $l2s --filter=gaussian:50+upper --filter=median:20 >> $b
gmt $m $l2s --filter=maxprob:75+lower >> $b
gmt $m $l2s --outgrid=/somefile=nf+divide:10+nan:-999+offset:6 >> $b
gmt $m $l2s --outgrid=/otherfile+offset:a >> $b
gmt $m $l2s --outgrid=/otherfile+scale:100  >> $b
gmt $m $l2s --increment=12+exact/15+number >> $b
gmt $m $l2s --script >> $b
gmt $m $l2s --secfilter=min --sector_filter=max >> $b
gmt $m $l2s --secfilter=average --secfilter=median >> $b
gmt $m $l2s --secfilter=mode+lower --secfilter=mode+upper >> $b
gmt $m $l2s --depth >> $b
gmt $m $l2s --toggle_registration --toggle >> $b

diff $a $b --strip-trailing-cr > fail
