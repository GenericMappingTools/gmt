#!/usr/bin/env bash
#
# Test grdcontour longopts translation.

m=grdcontour
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -An -A100+a10
--l2stranstest -A1+an -A1+apu -A1+apd
--l2stranstest -A1+c1/2+d+e+fHelvetica
--l2stranstest -A1+gred+i+jTL+n1/2
--l2stranstest -A1+N10/20+o+p0.5p,red
--l2stranstest -A1+r20+t/L/file+um
--l2stranstest -A1+v+w6+x+=xyz
--l2stranstest -C/some/file.cpt -C20,25,40,45
--l2stranstest -D%6.2f
--l2stranstest -Fl -Fr
--l2stranstest -Gd2p
--l2stranstest -GD1 -Gf/some/file
--l2stranstest -Gn6 -Gl1/2,3/4
--l2stranstest -GL1/2,3/4
--l2stranstest -Gx/x/file -GX/X/file
--l2stranstest -L1000/2000 -Ln -LN
--l2stranstest -Lp -LP
--l2stranstest -N/some/file.cpt
--l2stranstest -Q+z -Q20 -Q500m
--l2stranstest -S2 -S5
--l2stranstest -Th+a -Tl+d5/10+lLH
--l2stranstest -Wa2p,red -Wcthin+cl
--l2stranstest -Z+o5+p+s2
--l2stranstest -Z+p+s2
EOF

# module-specific longopts
gmt $m $l2s --annotation=none --annot=100+angle:10 >> $b
gmt $m $l2s --annot=1+angle:n --annot=1+angle:pu --annot=1+angle:pd >> $b
gmt $m $l2s --annot=1+clearance:1/2+debug+delay+font:Helvetica >> $b
gmt $m $l2s --annot=1+opaque:red+nolines+justify:TL+nudge:1/2 >> $b
gmt $m $l2s --annot=1+xynudge:10/20+round+outline:0.5p,red >> $b
gmt $m $l2s --annot=1+minradius:20+labelfile:/L/file+unit:m >> $b
gmt $m $l2s --annot=1+curved+npoints:6+add+prefix:xyz >> $b
gmt $m $l2s --contours=/some/file.cpt --interval=20,25,40,45 >> $b
gmt $m $l2s --dump='%6.2f' >> $b
gmt $m $l2s --force=left --force=right >> $b
gmt $m $l2s --label_placement=plotdist:2p >> $b
gmt $m $l2s --labels=mapdist:1 --labels=matchlocs:/some/file >> $b
gmt $m $l2s --labels=nlabels:6 --labels=lines:1/2,3/4 >> $b
gmt $m $l2s --labels=gcircles:1/2,3/4 >> $b
gmt $m $l2s --labels=xlines:/x/file --labels=xgcircles:/X/file >> $b
gmt $m $l2s --range=1000/2000 --limit=negative --limit=zeronegative >> $b
gmt $m $l2s --limit=positive --limit=zeropositive >> $b
gmt $m $l2s --fill=/some/file.cpt >> $b
gmt $m $l2s --cut+nozero --minpoints=20 --minlength=500m >> $b
gmt $m $l2s --smooth=2 --resample=5 >> $b
gmt $m $l2s --ticks=highs+all --ticks=lows+gap:5/10+labels:LH >> $b
gmt $m $l2s --pen=annotated:2p,red --pen=regular:thin+color:l >> $b
gmt $m $l2s --modify+offset:5+periodic+scale:2 >> $b
gmt $m $l2s --scale+periodic+scale:2 >> $b

diff $a $b --strip-trailing-cr > fail
