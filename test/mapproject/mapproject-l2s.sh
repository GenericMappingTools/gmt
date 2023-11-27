#!/usr/bin/env bash
#
# Test mapproject longopts translation.

m=mapproject
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Ab10/11 -AB+v -Af25/30
--l2stranstest -AF -Ao5/6 -AO1/1+v
--l2stranstest -C -C3/4 -C3/6+m
--l2stranstest -Dc -Di -Dp
--l2stranstest -E219 -EAiry:1,2,3 -E6377563.396,299.324964600
--l2stranstest -Fd -Fm -Fs -Fe
--l2stranstest -Ff -Fk -FM -Fn
--l2stranstest -Fu -Fc -Fi -Fp
--l2stranstest -G10/20 -G10/20+a+i
--l2stranstest -G10/20+uM -G+v
--l2stranstest -I
--l2stranstest -L/My/File.txt+p+uC
--l2stranstest -Na -Nc
--l2stranstest -Ng -Nm
--l2stranstest -Qd -Qe
--l2stranstest -S
--l2stranstest -Th219/Airy:1,2,3 -TAiry:1,2,3/Airy:4,5,6
--l2stranstest -Wg10/20 -Wh -WjRM
--l2stranstest -Z -Z0.5 -Z+a
--l2stranstest -Z+i -Z+f -Z+t45678901
EOF

# module-specific longopts
gmt $m $l2s --azimuth=back:10/11 --azimuth=backgeodetic+variable --azimuth=forward:25/30 >> $b
gmt $m $l2s --azimuth=forwardgeodetic --azimuth=orient:5/6 --azimuth=orientgeodetic:1/1+variable >> $b
gmt $m $l2s --center --center=3/4 --center=3/6+merclat >> $b
gmt $m $l2s --lengthunit=cm --lengthunit=inch --lengthunit=point >> $b
gmt $m $l2s --ecef=219 --ecef=Airy:1,2,3 --ecef=6377563.396,299.324964600 >> $b
gmt $m $l2s --projunit=deg --projunit=min --projunit=sec --projunit=meter >> $b
gmt $m $l2s --projunit=foot --projunit=km --projunit=smile --projunit=nmile >> $b
gmt $m $l2s --projunit=ussft --projunit=cm --projunit=inch --projunit=point >> $b
gmt $m $l2s --stride=10/20 --stride=10/20+accumulated+incremental >> $b
gmt $m $l2s --stride=10/20+unit:M --stride+variable >> $b
gmt $m $l2s --inverse >> $b
gmt $m $l2s --proximity=/My/File.txt+segmentpoint+unit:C >> $b
gmt $m $l2s --latconvert=authalic --latconvert=conformal >> $b
gmt $m $l2s --latconvert=geocentric --latconvert=meridional >> $b
gmt $m $l2s --projinfo=datums --projinfo=ellipsoids >> $b
gmt $m $l2s --suppress >> $b
gmt $m $l2s --changedatum=height:219/Airy:1,2,3 --changedatum=Airy:1,2,3/Airy:4,5,6 >> $b
gmt $m $l2s --mapinfo=plotcoords:10/20 --mapinfo=height --mapinfo=justify:RM >> $b
#gmt $m $l2s --mapinfo=bbox --mapinfo=bboxtext --mapinfo=encompass --mapinfo=encompasstext >> $b
#gmt $m $l2s --mapsize=proj --mapsize=projtext --mapsize=normalize >> $b
#gmt $m $l2s --mapsize=corner --mapsize=cornertext --mapsize=region --mapsize=regiontext >> $b
#gmt $m $l2s --mapsize=width --mapsize=xy+npoints:100/200 >> $b
gmt $m $l2s --traveltime --traveltime=0.5 --traveltime+accumulated >> $b
gmt $m $l2s --traveltime+incremental --traveltime+isoformat --traveltime+epochtime:45678901 >> $b

diff $a $b --strip-trailing-cr > fail
