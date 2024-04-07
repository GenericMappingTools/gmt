#!/usr/bin/env bash
#
# Test common longopts translation.

# Note the choice of module to use for this test
# is not particularly critical assuming that the
# the module used (i) has been coded for longopts
# translation and (ii) is not unusual with regard
# to command-line argument processing.
m=psxy
l2s='--l2stranstest'
c=common
a=$c-l2s-a.txt
b=$c-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -JC10/20/5+dh
--l2stranstest -Jg20/20/20000+a5+t2+v5/5+w3+z1
--l2stranstest -JP12+fe+kx+r1
--l2stranstest -R1/2/3/4+ud+r -R1/2/3/4+r+ud
--l2stranstest -Vc
--l2stranstest -a4=animal,5=vegetable,6=mineral
--l2stranstest -biL+b -biH+l
--l2stranstest -di-999+c4
--l2stranstest -eabc*xyz+fpatterns.txt
--l2stranstest -fi0x,1y -fo9T,10d
--l2stranstest -hi4+d -ho+c+mMySegHdr
--l2stranstest -ho3+rMyRemark+tMyTitle
--l2stranstest -i0:2:8+l -i13:15+d0.5
--l2stranstest -i3:2:7+o12 -i20:25+s6.9d,tMyWord
--l2stranstest -qi~0:2:6+a -qi1:9+t
--l2stranstest -qi10:26+s -qi50/+c12
--l2stranstest -rg -rp
--l2stranstest -wy -wa -ww -wd
--l2stranstest -wh -wm -ws+c5
--l2stranstest -wc5/1+c3
EOF

# common longopts
gmt $m $l2s --projection=C10/20/5+d:h >> $b
gmt $m $l2s --proj=g20/20/20000+a:5+t:2+v:5/5+w:3+z:1 >> $b
gmt $m $l2s --proj=P12+f:e+k:x+r:1 >> $b
gmt $m $l2s --region=1/2/3/4+unit:d+rect --limits=1/2/3/4+rectangular+unit:d >> $b
gmt $m $l2s --verbosity=c >> $b
gmt $m $l2s --aspatial=4=animal,5=vegetable,6=mineral >> $b
gmt $m $l2s --binary=in:L+bigendian --binary=in:H+littleendian >> $b
gmt $m $l2s --nodata=in:-999+column:4 >> $b
gmt $m $l2s --find='abc*xyz'+file:patterns.txt >> $b
gmt $m $l2s --coltypes=in:0x,1y --coltypes=out:9T,10d >> $b
gmt $m $l2s --header=in:4+delete --header=out+columns+header:MySegHdr >> $b
gmt $m $l2s --header=out:3+remark:MyRemark+title:MyTitle >> $b
gmt $m $l2s --incols=0:2:8+log10 --incols=13:15+divide:0.5 >> $b
gmt $m $l2s --incols=3:2:7+offset:12 --incols=20:25+scale:6.9d,tMyWord >> $b
gmt $m $l2s --inrows=~0:2:6+byset --inrows=1:9+bytable >> $b
gmt $m $l2s --inrows=10:26+bysegment --inrows=50/+column:12 >> $b
gmt $m $l2s --registration=gridline --registration=pixel >> $b
gmt $m $l2s --wrap=year --wrap=annual --wrap=week --wrap=day >> $b
gmt $m $l2s --wrap=hour --wrap=min --wrap=sec+column:5 >> $b
gmt $m $l2s --wrap=custom:5/1+column:3 >> $b

# Note that we do not test quasi-common longopts
# here as not all modules support them, hence an
# unfortunate choice for $module could result in
# test failure.

diff $a $b --strip-trailing-cr > fail
