#!/usr/bin/env bash
#
# Test pssolar longopts translation.

m=pssolar
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -C
--l2stranstest -Gred -Gp25+bgreen+fgray
--l2stranstest -GP3+bwhite+fred+r300
--l2stranstest -I+d2000-04-25T04:52+z02
--l2stranstest -I+z02:00
--l2stranstest -M
--l2stranstest -N
--l2stranstest -Td+d2000-04-25T04:52+z02
--l2stranstest -Tc+z02:00
--l2stranstest -Tn -Ta
--l2stranstest -W1p,blue,solid
EOF

# module-specific longopts
gmt $m $l2s --format >> $b
gmt $m $l2s --fill=red --fill=bit:25+bg:green+foreground:gray >> $b
gmt $m $l2s --fill=bitreverse:3+background:white+fg:red+dpi:300 >> $b
gmt $m $l2s --sun+date:2000-04-25T04:52+timezone:02 >> $b
gmt $m $l2s --sun+TZ:02:00 >> $b
gmt $m $l2s --dump >> $b
gmt $m $l2s --invert >> $b
gmt $m $l2s --terminators=daynight+date:2000-04-25T04:52+timezone:02 >> $b
gmt $m $l2s --terminators=civil+TZ:02:00 >> $b
gmt $m $l2s --terminators=nautical --terminators=astronomical >> $b
gmt $m $l2s --pen=1p,blue,solid >> $b

diff $a $b --strip-trailing-cr > fail
