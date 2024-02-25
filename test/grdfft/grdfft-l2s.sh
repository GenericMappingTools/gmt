#!/usr/bin/env bash
#
# Test grdfft longopts translation.

m=grdfft
l2s='--l2stranstest'
a=$m-l2s-a.txt
b=$m-l2s-b.txt
rm -f $a $b ; touch $b

cat << EOF > $a
--l2stranstest -A90 -A10
--l2stranstest -C12 -C-8
--l2stranstest -D5 -D
--l2stranstest -Er+n
--l2stranstest -Ex+wk -Ey
--l2stranstest -Fr1/2/3/4 -Fx10/20 -Fy4/8/1
--l2stranstest -G/some/file.grd=nf+d2+n-99
--l2stranstest -G/other/file.grd=nf+o6+s1.5
--l2stranstest -I4 -Ig
--l2stranstest -N10/20+d -Na+a
--l2stranstest -Nf+a+e
--l2stranstest -Nm+h+m
--l2stranstest -Nr+l+n
--l2stranstest -Ns+v+wfft+z
--l2stranstest -Q
--l2stranstest -S5 -Sd
EOF

# module-specific longopts
gmt $m $l2s --azimuth=90 --azim=10 >> $b
gmt $m $l2s --zcontinue=12 --upward=-8 >> $b
gmt $m $l2s --differentiate=5 --dfdz >> $b
gmt $m $l2s --power_spectrum=radial+normalize >> $b
gmt $m $l2s --power_spectrum=x+wavelength:k --power_spectrum=y >> $b
gmt $m $l2s --filter=isotropic:1/2/3/4 --filter=x:10/20 --filter=y:4/8/1 >> $b
gmt $m $l2s --outgrid=/some/file.grd=nf+divide:2+nan:-99 >> $b
gmt $m $l2s --outgrid=/other/file.grd=nf+offset:6+scale:1.5 >> $b
gmt $m $l2s --integrate=4 --integrate=gravity >> $b
gmt $m $l2s --dimensions=10/20+detrend --inquire=accurate+remove_mean >> $b
gmt $m $l2s --dimensions=actual+remove_mean+edge_point >> $b
gmt $m $l2s --dimensions=low_memory+remove_mid+edge_mirror >> $b
gmt $m $l2s --dimensions=rapid+leave_alone+no_extend >> $b
gmt $m $l2s --dimensions=show+verbose+suffix:fft+complex >> $b
gmt $m $l2s --no_wavenum_ops >> $b
gmt $m $l2s --scale=5 --scale=deflection >> $b

diff $a $b --strip-trailing-cr > fail
