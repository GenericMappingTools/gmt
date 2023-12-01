#!/usr/bin/env bash
#
# Test psxy longopts translation.

m=psxy
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Am -Ap -Ax
--l2stranstest -Ay -Ar -At
--l2stranstest -Csomecpt -Cred,green,blue -C#22aa33,#ff00ff,#0077ee
--l2stranstest -D -D12/24
--l2stranstest -Ex+a -Ey+A
--l2stranstest -EX+cf -EX+cl
--l2stranstest -EY+n -EY+w5p/1p
--l2stranstest -EY+p2p,black
--l2stranstest -Fca -Fcf
--l2stranstest -Fns -Fnr
--l2stranstest -Fp10/20
--l2stranstest -H5
--l2stranstest -I0.2 -I-0.8
--l2stranstest -L+b+xl+yb+p1p,blue
--l2stranstest -L+d+xr+yt -L+D+x6+y7
--l2stranstest -Nc -Nr
--l2stranstest -T
--l2stranstest -Z99 -Z/my/polygon/file.txt
EOF

# module-specific longopts
gmt $m $l2s --straightlines=mpfollow --straightlines=pmfollow --straightlines=xyalong >> $b
gmt $m $l2s --straightlines=yxalong --straightlines=rtalong --straightlines=tralong >> $b
gmt $m $l2s --cpt=somecpt --cmap=red,green,blue --cpt=#22aa33,#ff00ff,#0077ee >> $b
gmt $m $l2s --offset --offset=12/24 >> $b
gmt $m $l2s --errorbars=xbar+asymmetrical --errorbars=ybar+lhbounds >> $b
gmt $m $l2s --errorbars=boxwhisker+symbolfill:f --errorbars=boxwhisker+symbolfill:l >> $b
gmt $m $l2s --errorbars=stemleaf+notch --errorbars=stemleaf+capwidth:5p/1p >> $b
gmt $m $l2s --errorbars=stemleaf+pen:2p,black >> $b
gmt $m $l2s --connection=continuous:a --connection=continuous:f >> $b
gmt $m $l2s --connection=network:s --connection=network:r >> $b
gmt $m $l2s --connection=refpoint:10/20 >> $b
gmt $m $l2s --scale=5 >> $b
gmt $m $l2s --intensity=0.2 --intensity=-0.8 >> $b
gmt $m $l2s --polygon+bounds+xanchor:l+yanchor:b+pen:1p,blue >> $b
gmt $m $l2s --polygon+symdev+xanchor:r+yanchor:t --polygon+asymdev+xanchor:6+yanchor:7 >> $b
gmt $m $l2s --noclip=clipnorepeat --noclip=repeatnoclip >> $b
gmt $m $l2s --ignoreinfiles >> $b
gmt $m $l2s --zvalue=99 --level=/my/polygon/file.txt >> $b

diff $a $b --strip-trailing-cr > fail
