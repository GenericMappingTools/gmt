#!/usr/bin/env bash
#
# Test batch longopts translation.

m=batch
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -D -F
--l2stranstest -I -M -N
--l2stranstest -Qs
--l2stranstest -Sbpre.sh -Sfpost.sh
--l2stranstest -T6 -T50/100/5+n
--l2stranstest -T/some/timefile+p7+s12
--l2stranstest -T9+w -T"6+wmy string" -T2+W
--l2stranstest -W
--l2stranstest -Z
EOF

# module-specific longopts
gmt $m $l2s --noautomove --templatenames >> $b
gmt $m $l2s --include --onemaster --prefix >> $b
gmt $m $l2s --debug=noexec >> $b
gmt $m $l2s --optscript=preflight:pre.sh --optscript=postflight:post.sh >> $b
gmt $m $l2s --njobs=6 --minmax=50/100/5+njobs >> $b
gmt $m $l2s --timefile=/some/timefile+tagwidth:7+multisys:12 >> $b
gmt $m $l2s --njobs=9+words --njobs=6+words:'my string' --njobs=2+tabwords >> $b
gmt $m $l2s --tmpdir >> $b
gmt $m $l2s --erasescripts >> $b

diff $a $b --strip-trailing-cr > fail
