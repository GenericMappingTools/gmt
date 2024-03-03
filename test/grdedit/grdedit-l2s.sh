#!/usr/bin/env bash
#
# Test grdedit longopts translation.

m=grdedit
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A
--l2stranstest -Cb -Cc
--l2stranstest -Cn -Cp
--l2stranstest -G/some/file.grd=nf+d2+n-99
--l2stranstest -G/other/file.grd=nf+o6+s1.5
--l2stranstest -D+c/some.cpt+dsillydata+n-99
--l2stranstest -D"+c-+o1+rSmart Grid"
--l2stranstest -D"+s2+tBest Grid Ever+vmyvar"
--l2stranstest -D+xDr.X+yDr.Y+zDr.Z
--l2stranstest -Ea -Ee -Eh
--l2stranstest -El -Er
--l2stranstest -Et -Ev
--l2stranstest -L -L+n
--l2stranstest -L+p
--l2stranstest -N/some/table -N/other/file
--l2stranstest -S
--l2stranstest -T -T
EOF

# module-specific longopts
gmt $m $l2s --adjust_inc >> $b
gmt $m $l2s --cmdhist=both --command_history=current >> $b
gmt $m $l2s --cmdhist=none --command_history=previous >> $b
gmt $m $l2s --outgrid=/some/file.grd=nf+divide:2+nan:-99 >> $b
gmt $m $l2s --outgrid=/other/file.grd=nf+offset:6+scale:1.5 >> $b
gmt $m $l2s --header+cpt:/some.cpt+dname:sillydata+invalid:-99 >> $b
gmt $m $l2s --metadata+cpt:-+offset:1+remark:'Smart Grid' >> $b
gmt $m $l2s --header+scale:2+title:"Best Grid Ever"+varname:myvar >> $b
gmt $m $l2s --netcdf+xname:Dr.X+yname:Dr.Y+zname:Dr.Z >> $b
gmt $m $l2s --transform=hvflip --transform=xyswap --transform=hflip >> $b
gmt $m $l2s --transform=rot90ccw --transform=rot90cw >> $b
gmt $m $l2s --transform=transpose --transform=vflip >> $b
gmt $m $l2s --lonshift_numrange --lonshift_numrange+negative >> $b
gmt $m $l2s --lonshift_numrange+positive >> $b
gmt $m $l2s --nodes=/some/table --replace=/other/file >> $b
gmt $m $l2s --lonshift_region >> $b
gmt $m $l2s --toggle_registration --toggle >> $b

diff $a $b --strip-trailing-cr > fail
