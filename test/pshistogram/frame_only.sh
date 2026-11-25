#!/usr/bin/env bash
# Test that histogram without -R does not add annotations when -B only sets ticks, gridlines or the frame.
# Each panel's PostScript is checked for annotation strings like "(3) sw" and appended to frame_only.ps
# for visual inspection; $ps is not set, so there is no baseline image comparison.
printf '1\n2\n2\n3\n3\n3\n4\n' > t.txt
out=frame_only.ps
K=-K
panel () {	# panel <expected: none|some|xless> <title> <-B args...>
	local want=$1 title=$2 n; shift 2
	gmt pshistogram t.txt -T1 -Gred -JX4c "$@" -B+t"$title" $K $O $X > p.ps
	n=$(grep -cE '^\([0-9]+\) ' p.ps || true)
	case $want in
		none) [ "$n" -eq 0 ] || echo "$title: unexpected annotations" >> fail ;;
		some) [ "$n" -gt 0 ] || echo "$title: no annotations" >> fail; n_af=$n ;;
		xless) [ "$n" -gt 0 ] && [ "$n" -lt "$n_af" ] || echo "$title: x should have no annotations, y should" >> fail ;;
	esac
	cat p.ps >> $out
	O=-O; X=-X5.5c
}
rm -f $out
panel some  Baf      -Baf
panel none  Bf       -Bf
panel none  Bg       -Bg
panel none  B0       -B0
K=
panel xless "Bxf Bya" -Bxf -Bya
rm -f p.ps
