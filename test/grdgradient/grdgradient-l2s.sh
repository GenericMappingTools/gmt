#!/usr/bin/env bash
#
# Test grdgradient longopts translation.

m=grdgradient
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A20 -A20/30
--l2stranstest -Dacno
--l2stranstest -Dna
--l2stranstest -Em+a0.8 -Es+d0.3
--l2stranstest -Ep+p0.8+s20
--l2stranstest -Gfile.grd=nf+d2+n-99
--l2stranstest -Gother_file.grd=nf+o6+s1.5
--l2stranstest -Ne+a0.2+s4
--l2stranstest -Nt+o1
--l2stranstest -Qc+fThis/file -Qr+fThis/file
--l2stranstest -QR+fThat/file
--l2stranstest -Sa/b/cfile -Sd/e/ffile
EOF

# module-specific longopts
gmt $m $l2s --azimuth=20 --azim=20/30 >> $b
gmt $m $l2s --direction=aspect,conventional,add90,report >> $b
gmt $m $l2s --find_dir=add90,aspect >> $b
gmt $m $l2s --radiance=m+ambient:0.8 --lambert=simple+diffuse:0.3 >> $b
gmt $m $l2s --radiance=peucker+specular:0.8+shine:20 >> $b
gmt $m $l2s --outgrid=file.grd=nf+divide:2+nan:-99 >> $b
gmt $m $l2s --outgrid=other_file.grd=nf+offset:6+scale:1.5 >> $b
gmt $m $l2s --normalize=laplace+ambient:0.2+sigma:4 >> $b
gmt $m $l2s --norm=cauchy+offset:1 >> $b
gmt $m $l2s --tiles=save+file:This/file --save_stats=use+file:This/file >> $b
gmt $m $l2s --tiles=usedelete+file:That/file >> $b
gmt $m $l2s --slope_file=a/b/cfile --slopegrid=d/e/ffile >> $b

diff $a $b --strip-trailing-cr > fail
