#!/usr/bin/env bash
#
# Test gmt grdmask for proper handling of clobber modes -C.

# Make ID grids from a multipolygon file with IDs set to CPT entries
cat << EOF > mask.cpt
0	blue	1	- ;B
1	green	2	- ;G
2	red	3	- ;R
3	yellow	4	- ;Y
4	black	5	- ;K
EOF
# Instead of add another file I just swap the ID values for polygons 1 and 2 so we can test all modes
data=$(gmt which -Gc @multihole.gmt)
awk '{ if (NR == 10) {print "# @D2|\"polygon #1\""} else if (NR == 40) {print "# @D1|\"polygon #2\""} else {print $0}}' ${data}  > modes.gmt
gmt grdmask -R-3/8/-4/7 -I0.1 -r modes.gmt -aZ=ID -Nz -Gf.nc -fg -Cf
gmt grdmask -R-3/8/-4/7 -I0.1 -r modes.gmt -aZ=ID -Nz -Go.nc -fg -Co
gmt grdmask -R-3/8/-4/7 -I0.1 -r modes.gmt -aZ=ID -Nz -Gl.nc -fg -Cl
gmt grdmask -R-3/8/-4/7 -I0.1 -r modes.gmt -aZ=ID -Nz -Gu.nc -fg -Cu

gmt begin maskmode
	gmt subplot begin 2x2 -R-3/8/-4/7 -Fs8c/- -JQ8c -A+gwhite+p0.5p+o4p -M10p
	gmt subplot set -A"-Cf"
	gmt grdimage f.nc -Cmask.cpt
	gmt plot modes.gmt -W0.25p,white
	gmt subplot set -A"-Co"
	gmt grdimage o.nc -Cmask.cpt
	gmt plot modes.gmt -W0.25p,white
	gmt subplot set -A"-Cl"
	gmt grdimage l.nc -Cmask.cpt
	gmt plot modes.gmt -W0.25p,white
	gmt subplot set -A"-Cu"
	gmt grdimage u.nc -Cmask.cpt
	gmt plot modes.gmt -W0.25p,white
	gmt subplot end
	gmt colorbar -Cmask.cpt -DJBC -Li0.05i
gmt end show
