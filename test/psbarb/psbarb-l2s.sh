#!/usr/bin/env bash
#
# Test psbarb longopts translation.

m=psbarb
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -C/some/file -C/some/other/file
--l2stranstest -D1/2/3
--l2stranstest -Gred
--l2stranstest -I0.5 -I0.8
--l2stranstest -N -Nc
--l2stranstest -Nr -Nr
--l2stranstest -Q2+a100+ggreen+p-
--l2stranstest -Q6+jb+s3+w0.5
--l2stranstest -Q6+z
--l2stranstest -W0.1c,orange
EOF

# module-specific longopts
gmt $m $l2s --cpt=/some/file --cmap=/some/other/file >> $b
gmt $m $l2s --offset=1/2/3 >> $b
gmt $m $l2s --fill=red >> $b
gmt $m $l2s --intensity=0.5 --illumination=0.8 >> $b
gmt $m $l2s --noclip --noclip=clip_norepeat >> $b
gmt $m $l2s --noclip=repeat --noclip=noclip_repeat >> $b
gmt $m $l2s --barbs=2+angle:100+fill:green+pen:- >> $b
gmt $m $l2s --barbs=6+justify:b+longspeed:3+width:0.5 >> $b
gmt $m $l2s --barbs=6+uvdata >> $b
gmt $m $l2s --pen=0.1c,orange >> $b

diff $a $b --strip-trailing-cr > fail
