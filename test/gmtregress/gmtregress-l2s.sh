#!/usr/bin/env bash
#
# Test gmtregress longopts translation.

m=gmtregress
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A-20/20/1+fn -A-5/20/2+fp
--l2stranstest -C80 -C90
--l2stranstest -Ex -Ey -Eo
--l2stranstest -Eo -Er
--l2stranstest -Fxymrczw
--l2stranstest -Fwry
--l2stranstest -N1 -N2 -Nr
--l2stranstest -Nr -Nw -Nw
--l2stranstest -S -Sr
--l2stranstest -T5/100/2+i -T5+n
--l2stranstest -T/some/file
--l2stranstest -Wwxyr
--l2stranstest -Wry
--l2stranstest -Z+5 -Z-5
EOF

# module-specific longopts
gmt $m $l2s --angles=-20/20/1+force:n --slopes=-5/20/2+force:p >> $b
gmt $m $l2s --confidence=80 --confidence_level=90 >> $b
gmt $m $l2s --regression=x_on_y --regression=y_on_x --regression=ortho >> $b
gmt $m $l2s --regression_type=orthogonal --regression_type=reduced >> $b
gmt $m $l2s --columns=x,y,model,residual,symmetrical,standardized,weight >> $b
gmt $m $l2s --column_combination=weight,residual,y >> $b
gmt $m $l2s --norm=mean_absolute --norm=mean_squared --norm=lms >> $b
gmt $m $l2s --norm=LMS --norm=rms --norm=RMS >> $b
gmt $m $l2s --skip_outliers --restrict_outliers=reverse >> $b
gmt $m $l2s --range=5/100/2+inverse --range=5+number >> $b
gmt $m $l2s --series=/some/file >> $b
gmt $m $l2s --weighted=weights,sigmax,sigmay,correlations >> $b
gmt $m $l2s --weighted=correlations,sigmay >> $b
gmt $m $l2s --limit=+5 --limit=-5 >> $b

diff $a $b --strip-trailing-cr > fail
