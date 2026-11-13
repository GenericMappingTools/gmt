#!/usr/bin/env bash
# Test that -Z8 density integrates to 1 over unequal (log) bins and that -I reports its true min/max

cat << EOF > density_unequal_bins.txt
1.05
1.19
1.2
1.23
1.82
1.9
1.95
2.93
3.95
4.53
4.58
4.67
6.68
10.22
10.49
18.43
32.8
43.11
58.55
64.7
82.79
89.0
107.1
124.2
161.84
261.28
267.88
348.43
474.41
743.4
EOF

# Decade bins 1-10-100-1000: the bin starting at x is 9*x wide
gmt histogram density_unequal_bins.txt -T1/1000/1+l -Z8 -IO > bins.txt
gmt histogram density_unequal_bins.txt -T1/1000/1+l -Z8 -I  > range.txt

awk '
	NR == FNR {	# bins.txt: left edge in $1, density in $2
		area += $2 * 9 * $1
		if (FNR == 1 || $2 < lo) lo = $2
		if (FNR == 1 || $2 > hi) hi = $2
		next
	}
	{ rlo = $3; rhi = $4 }	# range.txt: xmin xmax ymin ymax
	function absd(a, b) { return (a > b) ? a - b : b - a }
	END {
		err = 0
		if (absd(area, 1.0) > 1e-9) {
			print "FAIL: sum of density*width =", area, "want 1"; err = 1
		}
		else
			print "PASS: sum of density*width =", area
		if (absd(rlo, lo) > 1e-9 || absd(rhi, hi) > 1e-9) {
			print "FAIL: -I y-range =", rlo, rhi, "but per-bin min/max =", lo, hi; err = 1
		}
		else
			print "PASS: -I y-range matches per-bin min/max:", rlo, rhi
		exit err
	}' bins.txt range.txt
