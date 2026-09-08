#!/usr/bin/env bash
#
# Test greenspline -A...+f6, which derives gradient constraints (midpoint,
# gradient, and azimuth) from successive x,y,z points along a profile or
# track.  We check that this matches feeding greenspline the equivalent
# -A...+f2 (x, y, gradient, azimuth) data, both for Cartesian (-Z1) and
# geographic (-Z2) input.  The profiles are deliberately diagonal so that a
# sign or convention error in either azimuth component changes the answer,
# and we assert the solution is not constant, since an all-NaN or flat grid
# would make the comparison vacuous.

# set AWK to awk if undefined
AWK=${AWK:-awk}

rm -f fail

# --- Cartesian case ------------------------------------------------------

cat << EOF > gspline_9_data.txt
0	0	0
4	0	4
0	-1.5	1
3.5	1.5	2
EOF

# Diagonal profile: legs bear 45 degrees, so both sin and cos of the azimuth matter
cat << EOF > gspline_9_profile.txt
0	-1	0
2	1	4
3	2	4
EOF

# Same information precomputed by hand into format 2:
# midpoint (1,0):     dz=4, ds=sqrt(8) -> gradient=1.4142135623730951, azimuth=45
# midpoint (2.5,1.5): dz=0, ds=sqrt(2) -> gradient=0,                  azimuth=45
cat << EOF > gspline_9_format2.txt
1	0	1.4142135623730951	45
2.5	1.5	0	45
EOF

gmt greenspline gspline_9_data.txt -R0/4/-2/2 -I0.5 -Sl -Z1 -Agspline_9_profile.txt+f6 -Ggspline_9_mode6.grd
gmt greenspline gspline_9_data.txt -R0/4/-2/2 -I0.5 -Sl -Z1 -Agspline_9_format2.txt+f2 -Ggspline_9_mode2.grd

# Guard against the vacuous case of an empty or constant grid
gmt grdinfo -C gspline_9_mode6.grd | $AWK '{if ($6 == $7) print "FAIL: Cartesian mode 6 grid is empty or constant: "$0}' >> fail

gmt grdmath gspline_9_mode6.grd gspline_9_mode2.grd SUB ABS = gspline_9_diff.grd
gmt grdinfo -C gspline_9_diff.grd | $AWK '{if ($6 > 1e-10 || $7 > 1e-10) print "FAIL: Cartesian +f6 differs from +f2: "$0}' >> fail

# --- Geographic case (exercises gmt_distance in km and the flat-Earth azimuth) ---

cat << EOF > gspline_9_gdata.txt
10	20	0
12	22	3
10.5	22	1
12	20	2
EOF

cat << EOF > gspline_9_gprofile.txt
10	20	0
11	21	5
12	21.5	5
EOF

# Build the format 2 equivalent with mapproject: -Af gives the forward azimuth
# from the previous point and -G+uk+i the incremental distance in km
gmt mapproject gspline_9_gprofile.txt -Af -G+uk+i -jf -fg --FORMAT_FLOAT_OUT=%.17g | \
	$AWK 'NR > 1 {printf "%.17g\t%.17g\t%.17g\t%.17g\n", 0.5*(px+$1), 0.5*(py+$2), ($3-pz)/$5, $4} {px = $1; py = $2; pz = $3}' > gspline_9_gformat2.txt

gmt greenspline gspline_9_gdata.txt -R10/12/20/22 -I0.25 -Sl -Z2 -Agspline_9_gprofile.txt+f6 -Ggspline_9_gmode6.grd -fg
gmt greenspline gspline_9_gdata.txt -R10/12/20/22 -I0.25 -Sl -Z2 -Agspline_9_gformat2.txt+f2 -Ggspline_9_gmode2.grd -fg

gmt grdinfo -C gspline_9_gmode6.grd | $AWK '{if ($6 == $7) print "FAIL: geographic mode 6 grid is empty or constant: "$0}' >> fail

gmt grdmath gspline_9_gmode6.grd gspline_9_gmode2.grd SUB ABS = gspline_9_gdiff.grd
gmt grdinfo -C gspline_9_gdiff.grd | $AWK '{if ($6 > 1e-8 || $7 > 1e-8) print "FAIL: geographic +f6 differs from +f2: "$0}' >> fail

# --- Segment handling: a 1-point segment yields no constraint but must not upset the count ---

cat << EOF > gspline_9_segs.txt
0	-1	0
2	1	4
>
3	2	4
EOF

gmt greenspline gspline_9_data.txt -R0/4/-2/2 -I0.5 -Sl -Z1 -Agspline_9_segs.txt+f6 -Ggspline_9_segs.grd -V 2>&1 | \
	grep "unique slope constraints" | $AWK '{if ($4 != 1) print "FAIL: expected 1 constraint from a 2-point plus 1-point segment file, got "$4}' >> fail
