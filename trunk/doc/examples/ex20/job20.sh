#!/bin/sh
#		GMT EXAMPLE 20
#
#		$Id: job20.sh,v 1.1 2001-09-14 18:55:04 pwessel Exp $
#
# Purpose:	Extend GMT to plot custom symbols
# GMT progs:	pscoast, psxy, mapproject
# Unix progs:	rm, $AWK
#
# Make volcano- and bullseye-symbol awk-scripts using
# make_symbol on the definition files volcano.def and
# bullseye.def

$AWK -f make_symbol < volcano.def > volcano.awk
$AWK -f make_symbol < bullseye.def > bullseye.awk

# Plot a world-map with volcano symbols of different sizes
# on top given locations and sizes in hotspots.d

cat << EOF > hotspots.d
55.5	-21.0	0.25
63.0	-49.0	0.25
-12.0	-37.0	0.25
-28.5	29.34	0.25
48.4	-53.4	0.25
155.5	-40.4	0.25
-155.5	19.6	0.5
-138.1	-50.9	0.25
-153.5	-21.0	0.25
-116.7	-26.3	0.25
-16.5	64.4	0.25
EOF

pscoast -R0/360/-90/90 -JR180/9i -B60/30:."Hotspot Islands and Cities": -G0/150/0 -S200/200/255 -Dc -A5000 -K -U"Example 20 in Cookbook" > example_20.ps

mapproject -R0/360/-90/90 -JR -Di hotspots.d | $AWK -f volcano.awk | psxy -R0/9/0/9 -Jx1i -O -K -M -W0.25p -G255/0/0 -L >> example_20.ps

# Overlay a few bullseyes at NY, Cairo, and Perth

cat << EOF > cities.d
286	40.45	0.8
31.15	30.03	0.8
115.49	-31.58	0.8
EOF

mapproject -R0/360/-90/90 -JR -Di cities.d | $AWK -f bullseye.awk | psxy -R0/9/0/9 -Jx -O -M >> example_20.ps

\rm -f volcano.awk bullseye.awk hotspots.d cities.d
