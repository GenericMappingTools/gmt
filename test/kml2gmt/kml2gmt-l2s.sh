#!/usr/bin/env bash
#
# Test kml2gmt longopts translation.

m=kml2gmt
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -E -E
--l2stranstest -Fs -Fl -Fp
--l2stranstest -Z
EOF

# module-specific longopts
gmt $m $l2s --z_ignore --extend_data >> $b
gmt $m $l2s --feature=point --feature_type=line --feature=polygon >> $b
gmt $m $l2s --altitudes >> $b

diff $a $b --strip-trailing-cr > fail
