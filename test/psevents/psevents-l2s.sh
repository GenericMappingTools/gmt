#!/usr/bin/env bash
#
# Test psevents longopts translation.

m=psevents
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -T1690329600s
--l2stranstest -Ar300i+v23 -As
--l2stranstest -Cmycpt
--l2stranstest -Dj20/10+v2p -DJ+v
--l2stranstest -Es+o0.1+rc2.1+p0.5+dl1+fq3
--l2stranstest -Et+O0.9+l0.5
--l2stranstest -F+a10+fHelvetica+jCM+r12+z%4.2f
--l2stranstest -H+c2/3+g#dd00ff+p2p+r+s1/1/gray
--l2stranstest -L -L5 -Lt
--l2stranstest -Mi+c7 -Ms -Mt -Mv3+c1
--l2stranstest -N -Nc -Nr
--l2stranstest -Qmyprefix
--l2stranstest -Sd0.5c
--l2stranstest -Wthin,red,-
--l2stranstest -Z"somemodule -a -b -c"
EOF

# module-specific longopts
gmt $m $l2s --time=1690329600s >> $b
gmt $m $l2s --polylines=trajectories:300i+value:23 --polylines=segments >> $b
gmt $m $l2s --cpt=mycpt >> $b
gmt $m $l2s --offset=justify:20/10+line:2p --offset=shortdiag+line >> $b
gmt $m $l2s --knots=symbol+offset:0.1+rise:c2.1+plateau:0.5+decay:l1+fade:q3 >> $b
gmt $m $l2s --knots=text+startoffset:0.9+text:0.5 >> $b
gmt $m $l2s --labels+angle:10+font:Helvetica+justify:CM+record:12+zvalue:%4.2f >> $b
gmt $m $l2s --boxes+clearance:2/3+fill:#dd00ff+pen:2p+rounded+shade:1/1/gray >> $b
gmt $m $l2s --length --length=5 --length=t >> $b
gmt $m $l2s --symbols=intensity+coda:7 --symbols=size --symbols=transparency --symbols=value:3+coda:1 >> $b
gmt $m $l2s --noclip --noclip=clipnorepeat --noclip=repeatnoclip  >> $b
gmt $m $l2s --save=myprefix >> $b
gmt $m $l2s --eventsymbol=d0.5c >> $b
gmt $m $l2s --pen=thin,red,- >> $b
gmt $m $l2s --symbolcommand='somemodule -a -b -c' >> $b

diff $a $b --strip-trailing-cr > fail
