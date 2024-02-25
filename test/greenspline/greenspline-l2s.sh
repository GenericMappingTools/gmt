#!/usr/bin/env bash
#
# Test greenspline longopts translation.

m=greenspline
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A/some/file+f0
--l2stranstest -Cn10+c+f/some/file
--l2stranstest -Cr6+i
--l2stranstest -Cv0.5+n
--l2stranstest -D+cmycpt -D+cmycpt
--l2stranstest -D+dbaddata+n-999+o5
--l2stranstest -D"+rsilly comment+s2+tBig News"
--l2stranstest -D+vDataset+xMrX+yMsY+zZeebo
--l2stranstest -E/some/file+r/report/file
--l2stranstest -G/some/file.grd
--l2stranstest -I4 -I2/3 -I1/2/3
--l2stranstest -L -Lt -Lr
--l2stranstest -N/node/file
--l2stranstest -Q -Q12 -Q4/5/6
--l2stranstest -Sc -Sl+e0.1
--l2stranstest -Sp -Sq200+n20
--l2stranstest -Sr200/10 -St500/3
--l2stranstest -T/this/mask.grd -T/that/mask.grd
--l2stranstest -W -Ww
--l2stranstest -Z3 -Z5
EOF

# module-specific longopts
gmt $m $l2s --gradient=/some/file+format:0 >> $b
gmt $m $l2s --approx_fit=largest:10+cumulative+file:/some/file >> $b
gmt $m $l2s --approx_fit=ratio:6+incremental >> $b
gmt $m $l2s --approximate=variance:0.5+no_surface >> $b
gmt $m $l2s --ncdata+cpt:mycpt --ncdata+cmap:mycpt >> $b
gmt $m $l2s --ncdata+dname:baddata+invalid:-999+offset:5 >> $b
gmt $m $l2s --ncdata+remark:'silly comment'+scale:2+title:'Big News' >> $b
gmt $m $l2s --ncdata+varname:Dataset+xname:MrX+yname:MsY+zname:Zeebo >> $b
gmt $m $l2s --misfit=/some/file+report:/report/file >> $b
gmt $m $l2s --outgrid=/some/file.grd >> $b
gmt $m $l2s --inc=4 --increment=2/3 --spacing=1/2/3 >> $b
gmt $m $l2s --leave_trend --detrend=leastsquares --detrend=residuals >> $b
gmt $m $l2s --nodes=/node/file >> $b
gmt $m $l2s --derivative --dir_derivative=12 --vector=4/5/6 >> $b
gmt $m $l2s --splines=min_scurvature --splines=linear+error:0.1 >> $b
gmt $m $l2s --splines=min_pcurvature --splines=ctensionA:200+npoints:20 >> $b
gmt $m $l2s --splines=rtension:200/10 --splines=ctensionB:500/3 >> $b
gmt $m $l2s --maskgrid=/this/mask.grd --mask=/that/mask.grd >> $b
gmt $m $l2s --uncertainties --uncertainties=weights >> $b
gmt $m $l2s --distmode=3 --mode=5 >> $b

diff $a $b --strip-trailing-cr > fail
