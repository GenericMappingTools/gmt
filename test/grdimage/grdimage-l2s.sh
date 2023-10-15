#!/usr/bin/env bash
#
# Test grdimage longopts translation.

m=grdimage
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -C/myfile.cpt+h12 -C/myfile.cpt+i10
--l2stranstest -Csomefile.cpt+ue -Csomefile.cpt+UM
--l2stranstest -D -Dr
--l2stranstest -Ei -E300
--l2stranstest -GSkyBlue+b -Ggray+f
--l2stranstest -I/somefile -I+d
--l2stranstest -I+a10+m0+nt1
--l2stranstest -M
--l2stranstest -N
--l2stranstest -Qred -Q+z-999
EOF

# module-specific longopts
gmt $m $l2s --cpt=/myfile.cpt+hinge:12 --cpt=/myfile.cpt+zinc:10 >> $b
gmt $m $l2s --cpt=somefile.cpt+fromunit:e --cpt=somefile.cpt+tounit:M >> $b
gmt $m $l2s --inimage --inimage=region >> $b
gmt $m $l2s --dpi=psdeviceres --dpi=300 >> $b
gmt $m $l2s --bitcolor=SkyBlue+background --bitcolor=gray+foreground >> $b
gmt $m $l2s --intensity=/somefile --intensity+default >> $b
gmt $m $l2s --intensity+azimuth:10+ambient:0+intensity:t1 >> $b
gmt $m $l2s --monochrome >> $b
gmt $m $l2s --noclip >> $b
gmt $m $l2s --alphacolor=red --alphacolor+gridvalue:-999 >> $b

diff $a $b --strip-trailing-cr > fail
