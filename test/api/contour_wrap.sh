#!/usr/bin/env bash
# Check the order of crossings when a contour wraps across a map boundary.
testapi_contour_wrap

# The same contour is drawn from matrices with different starting longitudes.
max_magenta_column () (
	set -o pipefail
	"$GRAPHICSMAGICK" convert "$1" -depth 8 txt:- | "$AWK" -F '[,[:space:]]+' '
		toupper($NF) == "#FF00FF" { if (++count[$1] > max) max = count[$1] }
		END { print max + 0 }'
)
matrix_peak=$(max_magenta_column matrix.png)
aligned_peak=$(max_magenta_column aligned.png)
if (( matrix_peak == 0 || aligned_peak == 0 || matrix_peak > 2 * aligned_peak )); then
	echo "Unexpected cropped contour peaks (matrix $matrix_peak, aligned $aligned_peak)" >&2
	return 1
fi
printf 'Cropped contour peaks (matrix %d, aligned %d)\n' "$matrix_peak" "$aligned_peak"
