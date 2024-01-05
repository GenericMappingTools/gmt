#!/usr/bin/env bash
#
# Test gmtconvert longopts translation.

m=gmtconvert
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A -A
--l2stranstest -C+l2+u5+i
--l2stranstest -D%09d+o6
--l2stranstest -Ef -El
--l2stranstest -Em -EM6
--l2stranstest -Fca -Fn10/20
--l2stranstest -Itsr
--l2stranstest -Irs
--l2stranstest -L -L
--l2stranstest -N2+a -N1+d
--l2stranstest -Q2,3:1:6 -Q+f/some/file
--l2stranstest -Q~3,4:2:12
--l2stranstest -S"some string+f/this/file+e"
--l2stranstest -S"~some other string+f/your/file"
--l2stranstest -Thd~3,4:2:12 -Th
--l2stranstest -W+n
--l2stranstest -Z
EOF

# module-specific longopts
gmt $m $l2s --horizontal --hcat >> $b
gmt $m $l2s --n_records+minrecs:2+maxrecs:5+invert >> $b
gmt $m $l2s --dump='%09d'+orig:6 >> $b
gmt $m $l2s --first_last=first --extract=last >> $b
gmt $m $l2s --first_last=stride --extract=stride_last:6 >> $b
gmt $m $l2s --conn_method=ca --conn_method=n10/20 >> $b
gmt $m $l2s --invert=tables,segments,records >> $b
gmt $m $l2s --reverse=records,segments >> $b
gmt $m $l2s --segment_headers --list_only >> $b
gmt $m $l2s --sort=2+ascend --sort=1+descend >> $b
gmt $m $l2s --segments=2,3:1:6 --segments+file:/some/file >> $b
gmt $m $l2s --segments=~3,4:2:12 >> $b
gmt $m $l2s --select_header='some string'+file:/this/file+exact >> $b
gmt $m $l2s --select_hdr=~'some other string'+file:/your/file >> $b
gmt $m $l2s --suppress=headers,duplicates:~3,4:2:12 --skip=headers >> $b
gmt $m $l2s --word2num+nonans >> $b
gmt $m $l2s --transpose >> $b

diff $a $b --strip-trailing-cr > fail
