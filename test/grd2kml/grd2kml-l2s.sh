#!/usr/bin/env bash
#
# Test grd2kml longopts translation.

m=grd2kml
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Aa100 -Ag -As
--l2stranstest -Csomecpt+h2+uin+s/some/file
--l2stranstest -C+Uin+i5
--l2stranstest -Ehttp://some.place.org/x.html
--l2stranstest -Fb -Fc
--l2stranstest -Fg -Fm
--l2stranstest -H1.5
--l2stranstest -I/my/illfile -I6+d
--l2stranstest -I+a90 -I6+m2+n3
--l2stranstest -L16 -L64
--l2stranstest -S2 -S3 -S4
--l2stranstest -TMyTitle
--l2stranstest -Wthin,red -W/file+s5/0.2
EOF

# module-specific longopts
gmt $m $l2s --mode=absolute:100 --mode=ground --mode=seafloor >> $b
gmt $m $l2s --cpt=somecpt+hinge:2+fromunit:in+file:/some/file >> $b
gmt $m $l2s --cmap+tounit:in+zinc:5 >> $b
gmt $m $l2s --url=http://some.place.org/x.html >> $b
gmt $m $l2s --filter=boxcar --filter=cosarch >> $b
gmt $m $l2s --filter=gaussian --filter=median >> $b
gmt $m $l2s --subpixel=1.5 >> $b
gmt $m $l2s --illumination=/my/illfile --intensity=6+default >> $b
gmt $m $l2s --shading+azimuth:90 --shading=6+ambient:2+nargs:3 >> $b
gmt $m $l2s --tilesize=16 --tile_size=64 >> $b
gmt $m $l2s --extra=2 --extralayers=3 --extra_layers=4 >> $b
gmt $m $l2s --title=MyTitle >> $b
gmt $m $l2s --pen=thin,red --contours=/file+scale:5/0.2 >> $b

diff $a $b --strip-trailing-cr > fail
