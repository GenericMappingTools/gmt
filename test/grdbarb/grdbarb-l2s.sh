#!/usr/bin/env bash
#
# Test grdbarb longopts translation.

m=grdbarb
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A
--l2stranstest -Cfile -Cother/file
--l2stranstest -Gred
--l2stranstest -I2/3 -Ix4/5
--l2stranstest -N
--l2stranstest -Q2+a100+ggreen+p-
--l2stranstest -Q6+jb+s3+w0.5
--l2stranstest -T
--l2stranstest -W0.1c,orange
--l2stranstest -Z
EOF

# module-specific longopts
gmt $m $l2s --polar >> $b
gmt $m $l2s --cpt=file --cmap=other/file >> $b
gmt $m $l2s --fill=red >> $b
gmt $m $l2s --increment=2/3 --inc=xmult:4/5 >> $b
gmt $m $l2s --noclip >> $b
gmt $m $l2s --barbs=2+angle:100+fill:green+pen:- >> $b
gmt $m $l2s --barbs=6+justify:b+longspeed:3+width:0.5 >> $b
gmt $m $l2s --adjust_azimuth >> $b
gmt $m $l2s --pen=0.1c,orange >> $b
gmt $m $l2s --theta_azimuths >> $b

diff $a $b --strip-trailing-cr > fail
