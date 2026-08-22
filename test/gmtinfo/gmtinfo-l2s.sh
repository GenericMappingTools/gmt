#!/usr/bin/env bash
#
# Test gmtinfo longopts translation.

m=gmtinfo
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Aa -At -As
--l2stranstest -C -C
--l2stranstest -D0.15/0.25
--l2stranstest -El -EL5
--l2stranstest -Eh0 -EH9
--l2stranstest -F -Fi -Fd
--l2stranstest -Ft
--l2stranstest -Ib1/2/3+e4/3 -Ie6+r0.1
--l2stranstest -If4/2+R2 -Ip4/2
--l2stranstest -Is1+r2/1
--l2stranstest -L
--l2stranstest -T4w+c3
EOF

# module-specific longopts
gmt $m $l2s --report=all --report=per_table --report=per_segment >> $b
gmt $m $l2s --columns --numeric >> $b
gmt $m $l2s --center=0.15/0.25 >> $b
gmt $m $l2s --get_record=min --get_record=minabs:5 >> $b
gmt $m $l2s --get_record=max:0 --get_record=maxabs:9 >> $b
gmt $m $l2s --counts --counts=totals --counts=segments >> $b
gmt $m $l2s --counts=segments_reset >> $b
gmt $m $l2s --inc=box:1/2/3+extend_box:4/3 --spacing=exact:6+adjust:0.1 >> $b
gmt $m $l2s --increment=fft:4/2+extend_region:2 --increment=override:4/2 >> $b
gmt $m $l2s --increment=surface:1+adjust:2/1 >> $b
gmt $m $l2s --common_limits >> $b
gmt $m $l2s --nearest_multiple=4w+column:3 >> $b

diff $a $b --strip-trailing-cr > fail
