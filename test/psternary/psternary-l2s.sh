#!/usr/bin/env bash
#
# Test psternary longopts translation.

m=psternary
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Cfile -Cother/file -Jz1i
--l2stranstest -Gred -Gp25+bgreen+fgray -Jz1i
--l2stranstest -GP3+bwhite+fred+r300 -Jz1i
--l2stranstest -L1/2/3 -L4/-/- -Jz1i
--l2stranstest -N -N -Jz1i
--l2stranstest -S-1 -S+2 -Sa3 -Jz1i
--l2stranstest -SA4 -Sc5 -Jz1i
--l2stranstest -SC6 -Sd7 -Jz1i
--l2stranstest -SD8 -Se1/2/3 -Jz1i
--l2stranstest -Sg4 -SG5 -Jz1i
--l2stranstest -Sh6 -SH7 -Jz1i
--l2stranstest -Si8 -Si9 -Jz1i
--l2stranstest -SI1 -Sj1/2/3 -Jz1i
--l2stranstest -Sj4/5/6 -SkSomething/20 -Jz1i
--l2stranstest -Sl4+tMyText+fHelv+jCM -Jz1i
--l2stranstest -Sn1 -SN2 -Jz1i
--l2stranstest -Sp3 -Sr4/5 -Jz1i
--l2stranstest -Sr+s -Jz1i
--l2stranstest -SR1/2/3 -SR4/5/6 -Jz1i
--l2stranstest -Ss7 -SS8 -Jz1i
--l2stranstest -St9 -ST1 -Jz1i
--l2stranstest -Sw1/2/3 -Sw+i4 -Jz1i
--l2stranstest -Sw+a5 -Sw+r10+p2p,red -Jz1i
--l2stranstest -Sx -Sy -Jz1i
--l2stranstest -W1p,blue,solid -Jz1i
EOF

# module-specific longopts
#
# psternary pre-scans its options array prior to longoption translation
# and will append -Jz1i if it does not see -M, meaning that we cannot
# offer any longoption equivalent for -M (e.g., --dump) or this pre-scan
# will fail to locate that equivalent and incorrectly append -Jz1i
#gmt $m $l2s --dump >> $b
gmt $m $l2s --cpt=file --cmap=other/file >> $b
gmt $m $l2s --fill=red --fill=bit:25+bg:green+foreground:gray >> $b
gmt $m $l2s --fill=bitreverse:3+background:white+fg:red+dpi:300 >> $b
gmt $m $l2s --labels=1/2/3 --vertex_labels=4/-/- >> $b
gmt $m $l2s --noclip --no_clip >> $b
gmt $m $l2s --symbol=xdash:1 --style=plus:2 --symbol=star:3 >> $b
gmt $m $l2s --symbol=star_area:4 --symbol=circle:5 >> $b
gmt $m $l2s --symbol=circle_area:6 --symbol=diamond:7 >> $b
gmt $m $l2s --symbol=diamond_area:8 --symbol=ellipse:1/2/3 >> $b
gmt $m $l2s --symbol=octagon:4 --symbol=octagon_area:5 >> $b
gmt $m $l2s --symbol=hexagon:6 --symbol=hexagon_area:7 >> $b
gmt $m $l2s --symbol=invtriangle:8 --symbol=inverted_tri:9 >> $b
gmt $m $l2s --symbol=invtriangle_area:1 --symbol=rotrectangle:1/2/3 >> $b
gmt $m $l2s --symbol=rotated_rec:4/5/6 --symbol=custom:Something/20 >> $b
gmt $m $l2s --symbol=letter:4+text:MyText+font:Helv+justify:CM >> $b
gmt $m $l2s --symbol=pentagon:1 --symbol=pentagon_area:2 >> $b
gmt $m $l2s --symbol=point:3 --symbol=rectangle:4/5 >> $b
gmt $m $l2s --symbol=rectangle+corners >> $b
gmt $m $l2s --symbol=roundrectangle:1/2/3 --symbol=roundrect:4/5/6 >> $b
gmt $m $l2s --symbol=square:7 --symbol=square_area:8 >> $b
gmt $m $l2s --symbol=triangle:9 --symbol=triangle_area:1 >> $b
gmt $m $l2s --symbol=wedge:1/2/3 --symbol=wedge+inner:4 >> $b
gmt $m $l2s --symbol=wedge+arc:5 --symbol=wedge+radial:10+pen:2p,red >> $b
gmt $m $l2s --symbol=cross --symbol=ydash >> $b
gmt $m $l2s --pen=1p,blue,solid >> $b

diff $a $b --strip-trailing-cr > fail
