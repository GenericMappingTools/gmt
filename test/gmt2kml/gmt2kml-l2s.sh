#!/usr/bin/env bash
#
# Test gmt2kml longopts translation.

m=gmt2kml
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -Aa25 -Ag
--l2stranstest -Asx1.5
--l2stranstest -C/some/file.cpt -C/some/file.cpt
--l2stranstest -D/some/file.dscpt
--l2stranstest -E+e+s
--l2stranstest -Fe -Fs -Ft
--l2stranstest -Fl -Fp -Fw
--l2stranstest -Gred -G+f
--l2stranstest -Ggreen+n -G+n
--l2stranstest -I- -Ihttp://someplace.edu/myicon.html
--l2stranstest -K
--l2stranstest -Lnm1,nm2 -LnmX,nmY
--l2stranstest -Nt -N5 -N"Some %d format string"
--l2stranstest -O
--l2stranstest -Qa23 -Qi10 -Qs0.5
--l2stranstest -Sc2 -Sn3
--l2stranstest -TmyTitle/some/folder
--l2stranstest -W2 -W+c -W+cl -W+cf
--l2stranstest -Z+a10/20 -Z+f3/5
--l2stranstest -Z+l2/3 -Z+o+v
EOF

# module-specific longopts
gmt $m $l2s --altitude_mode=absolute:25 --altitude_mode=relative_surface >> $b
gmt $m $l2s --altitude_mode=relative_floor:x1.5 >> $b
gmt $m $l2s --cpt=/some/file.cpt --cmap=/some/file.cpt >> $b
gmt $m $l2s --description=/some/file.dscpt >> $b
gmt $m $l2s --line_render+extrude+connect >> $b
gmt $m $l2s --feature=event --feature_type=symbol --feature=timespan >> $b
gmt $m $l2s --feature=line --feature=polygon --feature=wiggle >> $b
gmt $m $l2s --color=red --fill+fill >> $b
gmt $m $l2s --color=green+font --color+font >> $b
gmt $m $l2s --icon=- --icon=http://someplace.edu/myicon.html >> $b
gmt $m $l2s --continue >> $b
gmt $m $l2s --extended=nm1,nm2 --extra_data=nmX,nmY >> $b
gmt $m $l2s --name=text --feature_name=5 --name="Some %d format string" >> $b
gmt $m $l2s --overlay >> $b
gmt $m $l2s --wiggle=azimuth:23 --wiggle=fixed:10 --wiggle=scale:0.5 >> $b
gmt $m $l2s --scale=icon:2 --scale=label:3 >> $b
gmt $m $l2s --title=myTitle/some/folder >> $b
gmt $m $l2s --pen=2 --pen+color --pen+color:l --pen+color:f >> $b
gmt $m $l2s --attributes+altitude:10/20 --attrib+fade:3/5 >> $b
gmt $m $l2s --attrib+detail:2/3 --attrib+open+invisible >> $b

diff $a $b --strip-trailing-cr > fail
