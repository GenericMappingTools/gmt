#!/usr/bin/env bash
#
# Test grdfilter longopts translation.

m=grdfilter
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -D -D0
--l2stranstest -Fb25 -Fc100+h
--l2stranstest -Fg50+u -Ff20+l
--l2stranstest -Fo30 -Fm20+q0.5
--l2stranstest -Fp75+l -Fh25/5+c+u
--l2stranstest -Fl100 -FL100
--l2stranstest -Fu100 -FU100
--l2stranstest -G/somefile=nf+d10+n-999+o6
--l2stranstest -G/otherfile+oa
--l2stranstest -G/otherfile+s100+c
--l2stranstest -I12+e/15+n
--l2stranstest -Ni -Np -Nr
--l2stranstest -T -T
EOF

# module-specific longopts
gmt $m $l2s --distance --distance=0 >> $b
gmt $m $l2s --filter=boxcar:25 --filter=cosarch:100+highpass >> $b
gmt $m $l2s --filter=gaussian:50+upper --filter=custom:20+lower >> $b
gmt $m $l2s --filter=operator:30 --filter=median:20+quantile:0.5 >> $b
gmt $m $l2s --filter=mlprob:75+lower --filter=histogram:25/5+center+upper >> $b
gmt $m $l2s --filter=minall:100 --filter=minpos:100 >> $b
gmt $m $l2s --filter=maxall:100 --filter=maxneg:100 >> $b
gmt $m $l2s --outgrid=/somefile=nf+divide:10+nan:-999+offset:6 >> $b
gmt $m $l2s --outgrid=/otherfile+offset:a >> $b
gmt $m $l2s --outgrid=/otherfile+scale:100+gdal >> $b
gmt $m $l2s --increment=12+exact/15+number >> $b
gmt $m $l2s --nans=ignore --nans=coregnan --nans=anynan >> $b
gmt $m $l2s --toggle_registration --toggle >> $b

diff $a $b --strip-trailing-cr > fail
