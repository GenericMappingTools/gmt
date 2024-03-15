#!/usr/bin/env bash
#
# Test psxyz longopts translation.

m=psxyz
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Am -Ap
--l2stranstest -Ax -Ay
--l2stranstest -Ar -At
--l2stranstest -Csomecpt -Cred,green,blue
--l2stranstest -C#22aa33,#ff00ff,#0077ee
--l2stranstest -D -D12/24
--l2stranstest -Gred+z
--l2stranstest -H5
--l2stranstest -I0.2 -I-0.8
--l2stranstest -L+b+xl+yb+p1p,blue
--l2stranstest -L+d+xr+yt
--l2stranstest -L+D+x6+y7
--l2stranstest -Nc -Nr
--l2stranstest -Q -Q
--l2stranstest -W2p,red+cl+o1p -W1p+s
--l2stranstest -W2p+v -W2p+z
--l2stranstest -Z99+t
--l2stranstest -Zmy/polygon/file.txt+T
EOF

# module-specific longopts
gmt $m $l2s --straightlines=mpfollow --straight_lines=pmfollow >> $b
gmt $m $l2s --straight_line=xyalong --straightlines=yxalong >> $b
gmt $m $l2s --straightlines=rtalong --straightlines=tralong >> $b
gmt $m $l2s --cpt=somecpt --cmap=red,green,blue >> $b
gmt $m $l2s --cpt=#22aa33,#ff00ff,#0077ee >> $b
gmt $m $l2s --offset --offset=12/24 >> $b
gmt $m $l2s --fill=red+zvalue >> $b
gmt $m $l2s --scale=5 >> $b
gmt $m $l2s --intensity=0.2 --intens=-0.8 >> $b
gmt $m $l2s --polygon+bounds+xanchor:l+yanchor:b+pen:1p,blue >> $b
gmt $m $l2s --close+symdev+xanchor:r+yanchor:t >> $b
gmt $m $l2s --closed_polygon+asymdev+xanchor:6+yanchor:7 >> $b
gmt $m $l2s --noclip=clipnorepeat --no_clip=repeatnoclip >> $b
gmt $m $l2s --nosort --no_sort >> $b
gmt $m $l2s --pen=2p,red+color:l+offset:1p --pen=1p+spline >> $b
gmt $m $l2s --pen=2p+vector --pen=2p+zvalues >> $b
gmt $m $l2s --zvalue=99+transparency >> $b
gmt $m $l2s --level=my/polygon/file.txt+twocols >> $b

diff $a $b --strip-trailing-cr > fail
