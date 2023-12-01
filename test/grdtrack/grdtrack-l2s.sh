#!/usr/bin/env bash
#
# Test grdtrack longopts translation.

m=grdtrack
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Af -Ap -Am
--l2stranstest -Ar+l -AR
--l2stranstest -C27/3+a -C27/3/1.5+v
--l2stranstest -C30/5+d -C30/5/2+f12
--l2stranstest -C30/5+l -C30/5+r
--l2stranstest -DMyFile -D/some/other/file
--l2stranstest -E25/10+a-7+c -ELB+d
--l2stranstest -ECM+g+i0.5 -ERT,LB+l4
--l2stranstest -E3/2+n5000+o6 -ERT+r3.5
--l2stranstest -F+b+n -F+r+z12
--l2stranstest -G/My/Big/File.grd -G+lsomeListfile.txt
--l2stranstest -N
--l2stranstest -Sa+a -Sm+d -Sp
--l2stranstest -Sl+r -SL+ssavefile
--l2stranstest -Su -SU
--l2stranstest -T12+e -T+p
--l2stranstest -Z
EOF

# module-specific longopts
gmt $m $l2s --resample=keeporig --resample=pmfollow --resample=mpfollow >> $b
gmt $m $l2s --resample=equidistant+rhumb --resample=exactfit >> $b
gmt $m $l2s --crossprofile=27/3+alternate --crossprofile=27/3/1.5+wesn >> $b
gmt $m $l2s --crossprofile=30/5+deviant --crossprofile=30/5/2+fixed:12 >> $b
gmt $m $l2s --crossprofile=30/5+left --crossprofile=30/5+right >> $b
gmt $m $l2s --linefile=MyFile --dfile=/some/other/file >> $b
gmt $m $l2s --profile=25/10+azimuth:-7+connect --profile=LB+distance >> $b
gmt $m $l2s --profile=CM+degrees+incr:0.5 --profile=RT,LB+length:4 >> $b
gmt $m $l2s --profile=3/2+npoints:5000+origin:6 --profile=RT+radius:3.5 >> $b
gmt $m $l2s --critical+balance+negative --critical+rms+zvalue:12 >> $b
gmt $m $l2s --outgrid=/My/Big/File.grd --outgrid+list:someListfile.txt >> $b
gmt $m $l2s --noskip >> $b
gmt $m $l2s --stack=average+values --stack=median+deviations --stack=mode >> $b
gmt $m $l2s --stack=lower+residuals --stack=lowerpos+save:savefile >> $b
gmt $m $l2s --stack=upper --stack=upperneg >> $b
gmt $m $l2s --radius=12+report --radius+replace >> $b
gmt $m $l2s --zonly >> $b

diff $a $b --strip-trailing-cr > fail
