#!/usr/bin/env bash
#
# Test subplot longopts translation.

m=subplot
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Asometag+c1/2+gred+jBC
--l2stranstest -A+jMR+o1/2+p2p,blue+r+v
--l2stranstest -A+R+s1/2/gray+v
--l2stranstest -Cw1 -Cw2 -Ce3
--l2stranstest -Ce4 -Cn5 -Cn6
--l2stranstest -Cx7 -Cy8
--l2stranstest -D
--l2stranstest -Ff1/2+af+c1/2+gblue
--l2stranstest -Ff3/4+f3,1/1,2+pblack
--l2stranstest -Fs5/6+f3,1/1,2+pblue+wblack
--l2stranstest -Fs1/2+wblack -Fs7/8
--l2stranstest -M1/2/3/4
--l2stranstest -Srt+lmylbl+syourlbl -Sc+p
--l2stranstest -Scols:b+tc
--l2stranstest -T"My Overarching Heading"
EOF

# module-specific longopts
# (note 'gmt {begin,end}' bracketing as module not available in classic mode)
gmt begin
gmt $m $l2s --autolabel=sometag+clearance:1/2+fill:red+justify:BC >> $b
gmt $m $l2s --autolabel+anchor:MR+offset:1/2+pen:2p,blue+roman+vtag >> $b
gmt $m $l2s --autolabel+Roman+shaded:1/2/gray+vertical >> $b
gmt $m $l2s --clearance=w:1 --clearance=west:2 --clearance=e:3 >> $b
gmt $m $l2s --clearance=east:4 --clearance=n:5 --clearance=north:6 >> $b
gmt $m $l2s --clearance=x:7 --clearance=y:8 >> $b
gmt $m $l2s --defaults >> $b
gmt $m $l2s --dimensions=overall:1/2+scale:f+expand:1/2+fill:blue >> $b
gmt $m $l2s --dims=figsize:3/4+fractions:3,1/1,2+perimeter:black >> $b
gmt $m $l2s --dims=subplot:5/6+frac:3,1/1,2+outline:blue+dividers:black >> $b
gmt $m $l2s --dims=subsize:1/2+divlines:black --dims=panels:7/8 >> $b
gmt $m $l2s --margins=1/2/3/4 >> $b
gmt $m $l2s --share=rows:t+label:mylbl+label2:yourlbl --share=x+parallel >> $b
gmt $m $l2s --share=cols:b+row_title:c >> $b
gmt $m $l2s --title='My Overarching Heading' >> $b
gmt end

diff $a $b --strip-trailing-cr > fail
