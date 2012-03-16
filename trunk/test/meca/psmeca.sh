#!/bin/bash
#	$Id$
#
# Check psmeca for plotting beach balls

header "Test psmeca for plotting focal mechanisms (5)"

# Right lateral Strike Slip
echo 0.0 5.0 0.0 0 90 0 5 0 0   Right Strike Slip | psmeca -Sa2.5c -Gblack -R-1/4/0/6 -JM14c -P -B2 -K > $ps

# Left lateral Strike Slip
echo 2.0 5.0 0.0 0 90 180 5 0 0 Left Strike Slip | psmeca -Sa2.5c -Gblack -R -J -K -O >> $ps

# Thrust
echo 0.0 3.0 0.0 0 45 90 5 0 0  Thrust | psmeca -Sa2.5c -Gblack -R -J -K -O >> $ps
echo 2.0 3.0 0.0 45 45 90 5 0 0 Thrust | psmeca -Sa2.5c -Gblack -R -J -K -O >> $ps

# Normal
echo 0.0 1.0 0.0 0 45 -90 5 0 0  Normal | psmeca -Sa2.5c -Gblack -R -J -K -O >> $ps
echo 2.0 1.0 0.0 45 45 -90 5 0 0 Normal | psmeca -Sa2.5c -Gblack -R -J -K -O >> $ps

# Mixed
echo 3.4 0.6 0.0 10 35 129 5 0 0 Mixed  | psmeca -Sa2.5c -Gblack -R -J -O >> $ps

pscmp
