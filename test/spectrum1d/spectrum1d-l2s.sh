#!/usr/bin/env bash
#
# Test spectrum1d longopts translation.

m=spectrum1d
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Cacg
--l2stranstest -Cnopxy
--l2stranstest -D0.1 -D5
--l2stranstest -Lh -Lm
--l2stranstest -Nyoda
--l2stranstest -S64
--l2stranstest -T
--l2stranstest -W
EOF

# module-specific longopts
gmt $m $l2s --outputs=admittance,coherent,gain >> $b
gmt $m $l2s --outputs=noise,sqcoherency,phase,x,y >> $b
gmt $m $l2s --spacing=0.1 --sample_dist=5 >> $b
gmt $m $l2s --leave_trend=remove_mid --leave_trend=remove_mean >> $b
gmt $m $l2s --name=yoda >> $b
gmt $m $l2s --size=64 >> $b
gmt $m $l2s --multifile >> $b
gmt $m $l2s --wavelength >> $b

diff $a $b --strip-trailing-cr > fail
