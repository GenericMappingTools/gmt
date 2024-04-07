#!/usr/bin/env bash
#
# Test psrose longopts translation.

m=psrose
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A20+r
--l2stranstest -Cfile -Cother/file
--l2stranstest -D
--l2stranstest -Em+wfile
--l2stranstest -F
--l2stranstest -Gred -Gp25+bgreen+fgray
--l2stranstest -GP3+bwhite+fred+r300
--l2stranstest -I
--l2stranstest -Lwestlbl,eastlbl,southlbl,northlbl
--l2stranstest -M+a20+bA+c
--l2stranstest -M+c+e+gred
--l2stranstest -M+h1.5+je
--l2stranstest -M+mfc+n3/1
--l2stranstest -M+o0/89+p2p
--l2stranstest -M+q+s+t1/2
--l2stranstest -M+vi7
--l2stranstest -M+z2
--l2stranstest -N0 -N1
--l2stranstest -N2 -N2+p3p
--l2stranstest -Q0.2
--l2stranstest -S+a
--l2stranstest -T
--l2stranstest -W2p,red -Wv3p,green
--l2stranstest -Zu -Z5
EOF

# module-specific longopts
gmt $m $l2s --sector=20+rose >> $b
gmt $m $l2s --cpt=file --cmap=other/file >> $b
gmt $m $l2s --shift >> $b
gmt $m $l2s --vectors=mean+modefile:file >> $b
gmt $m $l2s --no_scale >> $b
gmt $m $l2s --fill=red --fill=bit:25+bg:green+foreground:gray >> $b
gmt $m $l2s --fill=bitreverse:3+background:white+fg:red+dpi:300 >> $b
gmt $m $l2s --inquire >> $b
gmt $m $l2s --labels=westlbl,eastlbl,southlbl,northlbl >> $b
gmt $m $l2s --vector_params+apex:20+begin:A+cpt >> $b
gmt $m $l2s --vector_params+cmap+end+fill:red >> $b
gmt $m $l2s --vector_params+shape:1.5+justify:e >> $b
gmt $m $l2s --vector_params+midpoint:fc+norm:3/1 >> $b
gmt $m $l2s --vector_params+oblique:0/89+pen:2p >> $b
gmt $m $l2s --vector_params+angles+xycoords+trim:1/2 >> $b
gmt $m $l2s --vector_params+polar_scale:i7 >> $b
gmt $m $l2s --vector_params+polar_convert:2 >> $b
gmt $m $l2s --distribution=meanstddev --vonmises=medianL1 >> $b
gmt $m $l2s --distribution=LMSscale --distribution=lmsscale+pen:3p >> $b
gmt $m $l2s --alpha=0.2 >> $b
gmt $m $l2s --norm+area >> $b
gmt $m $l2s --orientation >> $b
gmt $m $l2s --pen=2p,red --pen=vector:3p,green >> $b
gmt $m $l2s --scale=unity --scale=5 >> $b

diff $a $b --strip-trailing-cr > fail
