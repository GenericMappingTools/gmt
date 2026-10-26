#!/usr/bin/env bash
#
# Check that grdcontour puts the contours of a linear ramp exactly where the
# analytic answer says they are, for both gridline- and pixel-registered grids.
#
# The tracer reads the grid through an in-memory GDAL raster, and GDAL samples a
# raster at its cell centers while GMT keeps its values at the grid nodes.  The
# wrapper's geotransform therefore has to be shifted by half a cell for gridline
# registration and not at all for pixel registration.  A wrong shift, a wrong
# sign on the y increment (rows run north to south), or a line offset that
# ignores the grid pad all move every contour by half a cell or flip the grid,
# and that is what this test looks for.
#
#   z = x  ->  contours at x = 1,2,...,10, each spanning the full height
#   z = y  ->  contours at y = 1,2,...,10, each spanning the full width
#
# The gridline-registered grid runs from 0 to 10 in z so the 10 contour lands on
# its far edge and the 0 contour does not exist (every node is >= 0), which is 10
# lines.  The pixel-registered one only reaches 0.25 to 9.75, so it has 9.
#
# It also checks that no dumped point lies outside the area covered by the grid
# nodes: GDAL pads a raster with a virtual half-cell border repeating the
# outermost row/column, so a contour reaching the edge of the grid comes back
# with a half-cell stub sticking out of the domain unless it is trimmed off.

rm -f fail

# check <dump> <level column> <span column> <span min> <span max> <n segments>
check () {
	$AWK -v c=$2 -v o=$3 -v omin=$4 -v omax=$5 -v want=$6 -v tag="$1" '
	function report () {
		if (n == 0) return
		nseg++
		if (wmin > omin + 0.01 || wmax < omax - 0.01)
			printf "%s: level %g spans only %g/%g, expected %g/%g\n", tag, lev, wmin, wmax, omin, omax
		if (wmin < omin - 0.01 || wmax > omax + 0.01)
			printf "%s: level %g runs outside the node area: %g/%g\n", tag, lev, wmin, wmax
	}
	/^>/ { report(); n = 0; next }
	NF >= 3 {
		lev = $3
		d = $c - lev; if (d < 0) d = -d
		if (d > 0.005) printf "%s: level %g has a point %g off the level\n", tag, lev, d
		if (n == 0 || $o < wmin) wmin = $o
		if (n == 0 || $o > wmax) wmax = $o
		n++
	}
	END {	report()
		if (nseg != want) printf "%s: %d segments, expected %d\n", tag, nseg, want
	}' "$1"
}

# 1. Gridline registration: the nodes cover the whole -R, so the contours must too
gmt grdmath -R0/10/0/10 -I0.5 X = ramp_x.nc
gmt grdmath -R0/10/0/10 -I0.5 Y = ramp_y.nc
gmt grdcontour ramp_x.nc -C1 -JX10c -Dgrid_x.txt
gmt grdcontour ramp_y.nc -C1 -JX10c -Dgrid_y.txt
check grid_x.txt 1 2 0 10 10 >> fail
check grid_y.txt 2 1 0 10 10 >> fail

# 2. Pixel registration: the nodes sit half a cell inside -R, so the contours stop
#    half a cell short of it, which is exactly where gmt_contours also stopped
gmt grdmath -R0/10/0/10 -I0.5 -r X = pix_x.nc
gmt grdmath -R0/10/0/10 -I0.5 -r Y = pix_y.nc
gmt grdcontour pix_x.nc -C1 -JX10c -Dpix_x.txt
gmt grdcontour pix_y.nc -C1 -JX10c -Dpix_y.txt
check pix_x.txt 1 2 0.25 9.75 9 >> fail
check pix_y.txt 2 1 0.25 9.75 9 >> fail
