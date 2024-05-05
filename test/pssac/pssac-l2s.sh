#!/usr/bin/env bash
#
# Test pssac longopts translation.

m=pssac
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -C10/20
--l2stranstest -D20/30
--l2stranstest -Ea -Eb -Ek
--l2stranstest -Ed -En5 -Eu3
--l2stranstest -Fiqrii
--l2stranstest -Gp+gred+t0/5
--l2stranstest -Gn+z6
--l2stranstest -M25[m]/1.5
--l2stranstest -Q
--l2stranstest -S20 -Si200
--l2stranstest -T+t-5(b)+r0.5+s3
--l2stranstest -W1p,blue,solid
EOF

# module-specific longopts
gmt $m $l2s --timewindow=10/20 >> $b
gmt $m $l2s --offset=20/30 >> $b
gmt $m $l2s --profile=azimuth --profile=backazimuth --profile=epicenterkm >> $b
gmt $m $l2s --profile=epicenterdeg --profile=tracenum:5 --profile=userdef:3 >> $b
gmt $m $l2s --preprocess=iqrii >> $b
gmt $m $l2s --paint=positive+fill:red+timewindow:0/5 >> $b
gmt $m $l2s --paint=negative+zeroline:6 >> $b
gmt $m $l2s --vertscale=25[m]/1.5 >> $b
gmt $m $l2s --vertical >> $b
gmt $m $l2s --timescale=20 --timescale=inverse:200 >> $b
gmt $m $l2s --timeadjust+align:'-5(b)'+reducevel:0.5+shift:3 >> $b
gmt $m $l2s --pen=1p,blue,solid >> $b

diff $a $b --strip-trailing-cr > fail
