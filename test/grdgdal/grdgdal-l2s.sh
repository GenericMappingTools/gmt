#!/usr/bin/env bash
#
# Test grdgdal longopts translation.

m=grdgdal
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Adem+mhillside
--l2stranstest -Acolor-relief+c/some/file.cpt
--l2stranstest -Acolor-relief+c/other/file.cpt
--l2stranstest -G/some/file.grd=nf+d2+n-99
--l2stranstest -G/other/file.grd=nf+o6+s1.5
--l2stranstest -F"--welldone --withcheese"
--l2stranstest -M+r -M+w -M+r+w
EOF

# module-specific longopts
gmt $m $l2s --gdal_program=dem+method:hillside >> $b
gmt $m $l2s --gdal_program=color-relief+cpt:/some/file.cpt >> $b
gmt $m $l2s --gdal_program=color-relief+cmap:/other/file.cpt >> $b
gmt $m $l2s --outgrid=/some/file.grd=nf+divide:2+nan:-99 >> $b
gmt $m $l2s --outgrid=/other/file.grd=nf+offset:6+scale:1.5 >> $b
gmt $m $l2s --gdal_options="--welldone --withcheese" >> $b
gmt $m $l2s --gdal_io+read --gdal_io+write --gdal_io+read+write >> $b

diff $a $b --strip-trailing-cr > fail
