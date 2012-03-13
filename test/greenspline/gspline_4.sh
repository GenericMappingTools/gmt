#!/bin/bash
#
#       $Id$

. ./functions.sh
header "greenspline: Testing Cartesian 3-D interpolation"

# Figure 5 in Wessel, P. (2009), A general-purpose Green's function-based
#	interpolator, Computers & Geosciences, 35, 1247–1254.

T="$src"/Table_5_23.d
R3D=5/40/-5/10/5/16
R2D=12/32/0/6
Z=5/10
dz=0.25
view=200/25
method=r
tens=0.85
greenspline -R$R3D -I$dz -G3D.xyzw $T -S${method}${tens} -D5
k=0
rm -f total_dump
psbasemap -R$R2D/$Z -JX6/3 -JZ2.5 -p$view -B5f1g1/1g1/2f1WSneZ+ -P -K > $ps
psxyz -R -JX -JZ -p$view -O -K $T -Su0.05i -Gblack -Wfaint >> $ps
while [ $k -lt 22 ]; do
	z=`gmtmath -Q 5 $k $dz MUL ADD =`
#	echo "Doing z = $z"
	awk '{if ($3 == '$z') print $1, $2, $4}' 3D.xyzw | xyz2grd -R$R2D -I0.25/0.5 -Gslice_$k.nc
	grdcontour $Rcut slice_$k.nc -JX -C10 -L9/11 -S8 -Ddump
	awk '{if ($1 == ">") {print $0} else {print $1, $2, '$z'}}' dump > tmp
	cat tmp >> total_dump
	psxyz -R$R2D/$Z -JX -JZ -p$view -O -K tmp -Gp300/39:FgrayB- -Wthin >> $ps
	k=`expr $k + 1`
done
#minmax -M total_dump
echo "12 6 Volume exceeding 10% UO@-2@- concentration" | pstext -R$R2D/$Z -JX -JZ -p$view -F+jLT+f16p -O -K -Z10 -Dj0.1i/0.1i >> $ps
psxyz -R -JX -JZ -p$view -O -Wthin << EOF >> $ps
>
12 0 5
12 0 10
>
12 0 10
12 6 10
>
12 0 10
32 0 10
EOF

pscmp
