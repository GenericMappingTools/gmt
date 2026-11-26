#!/usr/bin/env bash
# Test gmt inset -Dj placement inside subplot panels (issue #6415).
# Panels: (a) -JQ? auto-scaled and centered vertically, (b) no -J so the default
# Cartesian -J is rescaled to the panel, (c) fixed -JQ8c, not rescaled nor
# centered, (d) -JQ? with subplot -C gaps.
# No baseline: in each panel a dot is plotted at the inset corner from inside the
# inset, and another one after inset end at the map corner shifted by the +o
# offset.  Both must land on the same spot of the PostScript page.
# gmtest gives any script that spells out the modern mode start command a $ps to
# compare, so we spell it via $start (as in subplot_3d_fit.sh).
start=begin
gmt $start insetpanels ps
	gmt subplot begin 2x2 -Fs8c/7c -Bf
		gmt basemap -R70/138/13/56 -JQ? -c0
		gmt inset begin -DjTL+w2c/1.5c+o0.2c -F+p1p,red
			echo 0 1 | gmt plot -R0/1/0/1 -JX2c/1.5c -Sc0.1c -Gred -N
		gmt inset end
		echo 70 56 | gmt plot -Sc0.1c -Gblue -N -D0.2c/-0.2c

		gmt basemap -R0/10/0/5 -c1
		gmt inset begin -DjBR+w2c/1.5c+o0.2c -F+p1p,red
			echo 1 0 | gmt plot -R0/1/0/1 -JX2c/1.5c -Sc0.1c -Gred -N
		gmt inset end
		echo 10 0 | gmt plot -Sc0.1c -Gblue -N -D-0.2c/0.2c

		gmt basemap -R70/138/13/56 -JQ8c -c2
		gmt inset begin -DjTR+w2c/1.5c+o0.2c -F+p1p,red
			echo 1 1 | gmt plot -R0/1/0/1 -JX2c/1.5c -Sc0.1c -Gred -N
		gmt inset end
		echo 138 56 | gmt plot -Sc0.1c -Gblue -N -D-0.2c/-0.2c

		gmt subplot set 3 -Cw1c -Cs0.5c
		gmt basemap -R70/138/13/56 -JQ?
		gmt inset begin -DjBL+w2c/1.5c+o0.2c -F+p1p,red
			echo 0 0 | gmt plot -R0/1/0/1 -JX2c/1.5c -Sc0.1c -Gred -N
		gmt inset end
		echo 70 13 | gmt plot -Sc0.1c -Gblue -N -D0.2c/0.2c
	gmt subplot end
gmt end
# Absolute page position of every circle: add up all translations (T and TM),
# saving/restoring them with the graphics state (V ... U).
LC_ALL=C awk '
	/^V( |$)/            {sx[++n] = x; sy[n] = y}
	/^U( |$)/            {x = sx[n]; y = sy[n--]}
	/^-?[0-9]+ -?[0-9]+ TM?$/ {x += $1; y += $2}
	/ Sc$/               {print x + $2, y + $3}
' insetpanels.ps > pos.txt
rm -f insetpanels.ps
# Each pair of lines (inset dot, map dot) must coincide within 2 PS units (1/600 inch)
LC_ALL=C awk 'NR % 2 {px = $1; py = $2; next}
	{dx = $1 - px; dy = $2 - py; if (dx*dx + dy*dy > 4) printf "panel %d: inset off by %d %d\n", NR/2, dx, dy}
	END {if (NR != 8) print "expected 8 circles, got", NR}' pos.txt > fail
