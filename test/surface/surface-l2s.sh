#!/usr/bin/env bash
#
# Test surface longopts translation.

m=surface
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -I0.1m+e/100+n
--l2stranstest -A0.6 -Am
--l2stranstest -C5 -C99.99%
--l2stranstest -D/some/file+z1000
--l2stranstest -Lld -Lusomefile.grd
--l2stranstest -M20c -M99.99
--l2stranstest -N25
--l2stranstest -Q -Qr
--l2stranstest -S25m
--l2stranstest -Tb0.5 -Ti0.1
--l2stranstest -W/a/b/c/mylog.txt
--l2stranstest -Z1.1234
EOF

# quasi-common longopts
gmt $m $l2s --increment=0.1m+exact/100+number >> $b

# module-specific longopts
gmt $m $l2s --aspect=0.6 --aspect=middle >> $b
gmt $m $l2s --convergence=5 --convergence=99.99% >> $b
gmt $m $l2s --breakline=/some/file+zvalue:1000 >> $b
gmt $m $l2s --limit=lower:d --limit=upper:somefile.grd >> $b
gmt $m $l2s --maskradius=20c --mask=99.99 >> $b
gmt $m $l2s --maxiterations=25 >> $b
gmt $m $l2s --quicksize --quicksize=region >> $b
gmt $m $l2s --searchradius=25m >> $b
gmt $m $l2s --tension=boundary:0.5 --tension=interior:0.1 >> $b
gmt $m $l2s --logfile=/a/b/c/mylog.txt >> $b
gmt $m $l2s --relax=1.1234 >> $b

diff $a $b --strip-trailing-cr > fail
