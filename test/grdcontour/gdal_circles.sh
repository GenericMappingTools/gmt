#!/usr/bin/env bash
#
# Contour z = x^2 + y^2, whose level L is exactly the circle of radius sqrt(L),
# and check the traced lines against that analytic answer.  This pins down the
# things a marching-squares tracer can get subtly wrong on a curved contour:
# the radius of every vertex, whether an interior contour comes back as a single
# closed ring, and whether -S densifies the line without moving it.
#
# The same grid is then punched with a NaN disk centered at (3.5,0) with radius
# 0.8, which the circles of levels 10 and 15 (radius 3.162 and 3.873) run
# straight through while those of levels 5 and 20 (2.236 and 4.472) miss.  The
# two affected levels must come back as a single open arc each, the other two
# must still be single closed rings, and no vertex may fall inside the hole.
#
# Interpolating a quadratic along a grid edge is off by at most h^2/4 = 6.3e-4
# in z, i.e. 1.4e-4 in radius on the tightest circle here, so 1e-3 is a tight
# tolerance that still catches an error of half a cell (0.025).

rm -f fail

gmt grdmath -R-5/5/-5/5 -I0.05 X SQR Y SQR ADD = r2.nc
gmt grdmath r2.nc X 3.5 SUB SQR Y SQR ADD SQRT 0.8 GE 0 NAN MUL = r2nan.nc

# radii <dump> <tag>: emit one line per level -
#   <tag> <level> <n segments> <closure pattern> <worst radial error> <vertices in the hole>
radii () {
	$AWK -v tag="$2" -v holed=$3 '
	function report () {
		if (n == 0) return
		nseg[lev]++
		gap = sqrt ((x0-xp)*(x0-xp) + (y0-yp)*(y0-yp))
		closed[lev] = closed[lev] (gap < 0.005 ? "C" : "O")
	}
	/^>/ { report(); n = 0; next }
	NF >= 3 {
		lev = $3
		r = sqrt ($1*$1 + $2*$2)
		d = r - sqrt (lev); if (d < 0) d = -d
		if (d > worst[lev]) worst[lev] = d
		if (holed && sqrt (($1-3.5)*($1-3.5) + $2*$2) < 0.79) hole[lev]++
		if (n == 0) { x0 = $1; y0 = $2 }
		xp = $1; yp = $2
		n++
	}
	END {	report()
		for (L in nseg)
			printf "%s %g %d %s %.6f %d\n", tag, L, nseg[L], closed[L], worst[L]+0, hole[L]+0
	}' "$1"
}

# expect <tag> <level> <n segments> <closure pattern> <radial tolerance>
expect () {
	$AWK -v tag="$1" -v L="$2" -v ns="$3" -v cl="$4" -v tol="$5" '
	$1 == tag && $2 == L {
		seen = 1
		if ($3 != ns)  printf "%s: level %s has %s segments, expected %s\n", tag, L, $3, ns
		if ($4 != cl)  printf "%s: level %s closure is %s, expected %s\n", tag, L, $4, cl
		if ($5 > tol)  printf "%s: level %s is off the analytic radius by %s (tol %s)\n", tag, L, $5, tol
		if ($6 != 0)   printf "%s: level %s has %s vertices inside the NaN hole\n", tag, L, $6
	}
	END { if (!seen) printf "%s: level %s is missing from the dump\n", tag, L }' summary.txt
}

# 1. Clean grid: four interior circles, one closed ring each
gmt grdcontour r2.nc -C5 -L5/20 -JX10c -Dclean.txt
radii clean.txt clean 0 > summary.txt

# 2. Same, smoothed 4x: must have far more points but stay on the same circles
gmt grdcontour r2.nc -C5 -L5/20 -S4 -JX10c -Dsmooth.txt
radii smooth.txt smooth 0 >> summary.txt

# 3. NaN-holed grid
gmt grdcontour r2nan.nc -C5 -L5/20 -JX10c -Dnan.txt
radii nan.txt nan 1 >> summary.txt

for t in clean smooth; do
	expect $t 5  1 C 0.001 >> fail
	expect $t 10 1 C 0.001 >> fail
	expect $t 15 1 C 0.001 >> fail
	expect $t 20 1 C 0.001 >> fail
done

# Cutting a closed ring once leaves ONE open arc.  Its two ends stop on the last
# cell that still had four valid corners, so they can sit up to half a cell off
# the analytic radius - hence the looser tolerance on the two cut levels.
expect nan 5  1 C 0.001 >> fail
expect nan 10 1 O 0.03  >> fail
expect nan 15 1 O 0.03  >> fail
expect nan 20 1 C 0.001 >> fail

# 4. -S4 must densify the lines about fourfold
$AWK 'END { print NR }' clean.txt  > n_clean.txt
$AWK 'END { print NR }' smooth.txt > n_smooth.txt
$AWK 'NR == FNR { a = $1; next } { if ($1 < 3 * a) printf "-S4 produced %d lines from %d, expected at least 3x\n", $1, a }' \
	n_clean.txt n_smooth.txt >> fail
