#!/usr/bin/env bash
#
# Check the option that decide WHICH levels get traced and in which direction.
#
# On the GDAL path grdcontour pre-filters the level list before handing it to
# the tracer, so that levels -L or -Q+z would throw away are not traced at all.
# That filter has to agree exactly with the tests at the top of the contour loop
# or levels go missing (or reappear) without any other symptom, so here we pin
# the level set each selector must produce.
#
# z = x on -R-9.5/9.5 with -C2 gives the nine levels -8,-6,...,8, none of them
# sitting on the domain boundary, and each one is a single vertical line.
#
# -F is checked on a closed ring, where flipping the orientation has to flip the
# sign of the signed area while leaving its magnitude alone.

rm -f fail

gmt grdmath -R-9.5/9.5/-5/5 -I0.5 X = ramp.nc

# levels <dump>: the sorted, unique level set of a dump, space separated.  The
# segment header is "> <level> contour -Z<level>", so the level is the -Z field.
levels () { $AWK '/^>/ { s = $NF; sub (/^-Z/, "", s); print s }' "$1" | sort -n -u | tr '\n' ' '; }

# want <tag> <dump> <expected level set>
want () {
	got=$(levels "$2")
	test "$got" = "$3 " || echo "$1: levels [$got] expected [$3]" >> fail
	n=$($AWK '/^>/ { n++ } END { print n+0 }' "$2")
	m=$($AWK -v s="$3" 'BEGIN { print split (s, a, " ") }')
	test "$n" -eq "$m" || echo "$1: $n segments for $m levels, expected one each" >> fail
}

gmt grdcontour ramp.nc -C2       -JX10c -Dall.txt
gmt grdcontour ramp.nc -C2 -Ln   -JX10c -Dln.txt
gmt grdcontour ramp.nc -C2 -LN   -JX10c -DlN.txt
gmt grdcontour ramp.nc -C2 -LP   -JX10c -DlP.txt
gmt grdcontour ramp.nc -C2 -Lp   -JX10c -Dlp.txt
gmt grdcontour ramp.nc -C2 -L-5/5 -JX10c -Drange.txt
gmt grdcontour ramp.nc -C2 -Q+z  -JX10c -Dqz.txt

want all   all.txt   "-8 -6 -4 -2 0 2 4 6 8"
want Ln    ln.txt    "-8 -6 -4 -2"
want LN    lN.txt    "-8 -6 -4 -2 0"
want LP    lP.txt    "0 2 4 6 8"
want Lp    lp.txt    "2 4 6 8"
want L-5/5 range.txt "-4 -2 0 2 4"
want Q+z   qz.txt    "-8 -6 -4 -2 2 4 6 8"

# -F: same ring, opposite winding
gmt grdmath -R-5/5/-5/5 -I0.05 X SQR Y SQR ADD = r2.nc
gmt grdcontour r2.nc -C5 -L5/5 -Fl -JX10c -Dleft.txt
gmt grdcontour r2.nc -C5 -L5/5 -Fr -JX10c -Dright.txt

# area <dump>: twice the signed area of the first segment, by the shoelace formula
area () { $AWK '/^>/ { if (n) exit; next } NF >= 3 { if (n) a += xp*$2 - $1*yp; xp = $1; yp = $2; n++ } END { printf "%.6f\n", a }' "$1"; }

al=$(area left.txt)
ar=$(area right.txt)
$AWK -v l="$al" -v r="$ar" 'BEGIN {
	if (l == 0 || r == 0) { print "-F: a signed area came out zero"; exit }
	if (l * r > 0) printf "-F: -Fl and -Fr wind the same way (%g and %g)\n", l, r
	d = (l < 0 ? -l : l) - (r < 0 ? -r : r); if (d < 0) d = -d
	if (d > 0.001 * (l < 0 ? -l : l)) printf "-F: -Fl and -Fr enclose different areas (%g and %g)\n", l, r
}' >> fail
