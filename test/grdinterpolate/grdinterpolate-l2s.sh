#!/usr/bin/env bash
#
# Test grdinterpolate longopts translation.

m=grdinterpolate
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -G/some/file.grd
--l2stranstest -D+xsomexnm[km]+ysomeynm[cm]+zsomeznm[nm]+dsomednm[km]
--l2stranstest -D+c-+s2+o100+n-999
--l2stranstest -D"+tTitle+rThis is a remark+vsomevar"
--l2stranstest -Efile+a-5+g+i1.5+l12+n1000
--l2stranstest -E1/2/3/4,5/6/7/8+o6+p+r7+x
--l2stranstest -Fl -Fa+d1
--l2stranstest -Fc -Fn+d2
--l2stranstest -S2/3+hsomehdr
--l2stranstest -T100/200/10+i -T10+n -T/some/file
--l2stranstest -Z -Z100/200/10+i -Z10+n
EOF

# module-specific longopts
gmt $m $l2s --outgrid=/some/file.grd >> $b
gmt $m $l2s --meta+xname:somexnm[km]+yname:someynm[cm]+zname:someznm[nm]+dname:somednm[km] >> $b
gmt $m $l2s --meta+cpt:-+scale:2+offset:100+invalid:-999 >> $b
gmt $m $l2s --meta+title:Title+remark:'This is a remark'+varname:somevar >> $b
gmt $m $l2s --profile=file+azimuth:-5+degrees+increment:1.5+length:12+npoints:1000 >> $b
gmt $m $l2s --crosssection=1/2/3/4,5/6/7/8+origin:6+parallel+radius:7+rhumb >> $b
gmt $m $l2s --interptype=linear --interptype=akima+derivative:1 >> $b
gmt $m $l2s --interptype=cubic --interptype=none+derivative:2 >> $b
gmt $m $l2s --pointseries=2/3+header:somehdr >> $b
gmt $m $l2s --inc=100/200/10+inverse --inc=10+numcoords --range=/some/file >> $b
gmt $m $l2s --levels --levels=100/200/10+inverse --levels=10+numcoords >> $b

diff $a $b --strip-trailing-cr > fail
