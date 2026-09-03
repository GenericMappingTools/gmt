#!/usr/bin/env bash
#
# Ghost cells (issue #4358): the boundary halo held outside the data matrix must give
# bit-identical answers to the legacy padded layout.  GMT_GHOST_CELLS selects the layout:
# 0 = legacy pad, 1 = grids move to ghost cells once their BCs are set, 2 = as 1 and no
# grid is created with a pad at all.  A ghost cell holds the same number the pad cell
# held, so nothing here is a tolerance test: the files must be identical.
#
# No baseline PostScript is needed; the answers are compared against the layout GMT
# has always used, computed by this same script.

gmt grdmath -R0/10/0/10 -I0.25 X Y MUL SIN = t.nc
# The corners are the interesting points: sampling there reads the halo and nothing else
printf "0 0\n10 10\n0 10\n10 0\n5.125 5.125\n0.05 9.95\n" > p.txt

for level in 0 1 2; do
	export GMT_GHOST_CELLS=$level
	gmt grdtrack p.txt -Gt.nc > track_$level.txt
	gmt grdgradient t.nc -A45 -Ne0.6 -Ggrad_$level.nc
	gmt grd2xyz grad_$level.nc > grad_$level.txt
	gmt grdsample t.nc -I0.1 -Gsamp_$level.nc
	gmt grd2xyz samp_$level.nc > samp_$level.txt
	gmt grdproject t.nc -Jm1i -Gproj_$level.nc
	gmt grd2xyz proj_$level.nc > proj_$level.txt
done
unset GMT_GHOST_CELLS

rm -f fail
for level in 1 2; do
	diff track_0.txt track_$level.txt >> fail
	diff grad_0.txt grad_$level.txt >> fail
	diff samp_0.txt samp_$level.txt >> fail
	diff proj_0.txt proj_$level.txt >> fail
done

# The layout must actually be live: an early version of the guard silently turned the
# whole thing into a no-op and everything above passed for the wrong reason
GMT_GHOST_DEBUG=1 GMT_GHOST_CELLS=1 gmt grdtrack p.txt -Gt.nc 2>&1 >/dev/null | grep -q "moved to ghost cells" \
	|| echo "ghost cells not used at GMT_GHOST_CELLS=1" >> fail
GMT_GHOST_DEBUG=1 GMT_GHOST_CELLS=2 gmt grdtrack p.txt -Gt.nc 2>&1 >/dev/null | grep -q "without ever holding a pad" \
	|| echo "grid still built with a pad at GMT_GHOST_CELLS=2" >> fail
