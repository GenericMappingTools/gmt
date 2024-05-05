#!/usr/bin/env bash
#
# Test sample1d longopts translation.

m=sample1d
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Af -Ap -Am
--l2stranstest -Ar+l -AR+d
--l2stranstest -Csomecpt+h2+uin+sfile
--l2stranstest -C+Uin+i5
--l2stranstest -E
--l2stranstest -Fl -Fe -Fa+d1
--l2stranstest -Fc -Fs -Fn
--l2stranstest -N1 -N2 -N3
--l2stranstest -T100/200/10+i -T10+n -Tfile
--l2stranstest -T10/20/1+a+u
--l2stranstest -W1
EOF

# module-specific longopts
gmt $m $l2s --resample=keeporig --resample=pmfollow --resample=mpfollow >> $b
gmt $m $l2s --resample=equidistant+rhumb --resample=exactfit+delete >> $b
gmt $m $l2s --cpt=somecpt+hinge:2+fromunit:in+file:file >> $b
gmt $m $l2s --cmap+tounit:in+zinc:5 >> $b
gmt $m $l2s --keeptext >> $b
gmt $m $l2s --interptype=linear --interp=step --interp=akima+derivative:1 >> $b
gmt $m $l2s --interptype=cubic --interptype=smooth --interptype=none >> $b
gmt $m $l2s --time_column=1 --time_col=2 --timecol=3 >> $b
gmt $m $l2s --inc=100/200/10+inverse --inc=10+numcoords --range=file >> $b
gmt $m $l2s --inc=10/20/1+paste+unique >> $b
gmt $m $l2s --weights=1 >> $b

diff $a $b --strip-trailing-cr > fail
