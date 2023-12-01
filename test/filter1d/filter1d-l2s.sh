#!/usr/bin/env bash
#
# Test filter1d longopts translation.

m=filter1d
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -D5 -D0.5
--l2stranstest -E
--l2stranstest -Fb25 -FB100
--l2stranstest -Fc25 -FC100+h
--l2stranstest -Fg50+u -FG20+l
--l2stranstest -Ff50+u -FF20+l
--l2stranstest -Fm20 -FM20
--l2stranstest -Fp75+l -FP25/5+u
--l2stranstest -Fl100 -FL100
--l2stranstest -Fu100 -FU100
--l2stranstest -L15 -N3 -N5
--l2stranstest -Q0.4 -S0.99
--l2stranstest -T10/100/5+a -T/some/file
--l2stranstest -T5+e -T5+i -T5+n
EOF

# module-specific longopts
gmt $m $l2s --increment=5 --inc=0.5 >> $b
gmt $m $l2s --end >> $b
gmt $m $l2s --filter=boxcar:25 --filter_type=rboxcar:100 >> $b
gmt $m $l2s --filter=cosarch:25 --filter_type=rcosarch:100+highpass >> $b
gmt $m $l2s --filter=gaussian:50+upper --filter=rgaussian:20+lower >> $b
gmt $m $l2s --filter=custom:50+upper --filter=rcustom:20+lower >> $b
gmt $m $l2s --filter=median:20 --filter=rmedian:20 >> $b
gmt $m $l2s --filter=mlprob:75+lower --filter=rmlprob:25/5+upper >> $b
gmt $m $l2s --filter=minall:100 --filter=minpos:100 >> $b
gmt $m $l2s --filter=maxall:100 --filter=maxneg:100 >> $b
gmt $m $l2s --gap_width=15 --time_col=3 --time_column=5 >> $b
gmt $m $l2s --quality=0.4 --symmetry=0.99 >> $b
gmt $m $l2s --range=10/100/5+array --range=/some/file >> $b
gmt $m $l2s --series=5+exact  --series=5+inverse --series=5+number >> $b

diff $a $b --strip-trailing-cr > fail
