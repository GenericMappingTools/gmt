#!/usr/bin/env bash
# Test that -Ra (auto-determine region from data) never rounds a bound on a
# log10 axis to a non-positive value, which is invalid on that axis (#6614).

gmt begin
gmt math -T0/10240/1 T 10240 DIV 360 MUL 400 MUL COSD = t.txt
gmt spectrum1d t.txt -S256 -W --GMT_FFT=brenner -N -i1 > pow5.txt
gmt plot pow5.txt -Ra -JX-15cl/4c -Bxa2f3 -Bya -Vd > plot.log 2>&1
gmt end

region=$(sed -n 's/.*Modern: Adding -R\([^ ]*\) to options\..*/\1/p' plot.log)
xlo=$(cut -d/ -f1 <<< "$region")
if [ -z "$region" ]; then
	echo "No -R was determined for -Ra" > fail
elif ! awk -v xlo="$xlo" 'BEGIN {exit !(xlo+0 > 0)}'; then
	echo "-Ra gave a non-positive x-min ($xlo) for a log10 x-axis: -R$region" > fail
fi
