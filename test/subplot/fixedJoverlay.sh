#!/usr/bin/env bash
# In a subplot panel whose map used a fixed-size -J, a module that takes -J from
# history must place a symbol at the same spot as one given the same -J explicitly.
# No baseline: we compare the final position (module origin + symbol) in the PS.
# gmtest gives any script that spells out the modern mode start command a $ps to compare,
# so we spell it via $start (as in subplot_3d_fit.sh).
start=begin
gmt $start fixedJoverlay ps
	gmt subplot begin 1x1 -Fs15c/10c -B0
		gmt coast -R10W/5E/35N/44N -JM15c -W1/faint -Dl -c0
		echo -9.14 38.72 | gmt plot -Sc0.3c -Gred -W0.5p -JM15c
		echo -9.14 38.72 | gmt plot -Sc0.3c -Gred -W0.5p
	gmt subplot end
gmt end
# Print x y of each plotted circle: origin shift (last TM before the module) + symbol position
awk '/ TM$/ {tx = $1; ty = $2} /^%@GMT: gmt plot/ {x0 = tx; y0 = ty} / Sc$/ {print x0 + $2, y0 + $3}' fixedJoverlay.ps > pos.txt
rm -f fixedJoverlay.ps
uniq pos.txt | awk 'END {if (NR != 1) print "circles not at the same place:", NR, "positions"}' > fail
