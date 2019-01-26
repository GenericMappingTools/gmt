#!/usr/bin/env bash
#		GMT Appendix P, example 1
#
# Purpose:	Illustrates the use of isolation mode
# GMT progs:	gmt set, gmt grdimage, gmt grdmath, gmt makecpt, gmt pscoast
# Unix progs:	mktemp, rm
#
ps=GMT_App_P_1.ps

# Create a temporary directory. $GMT_TMPDIR will be set to its pathname.
# XXXXXX is replaced by a unique random combination of characters.
export GMT_TMPDIR=`mktemp -d /tmp/gmt.XXXXXX`

# These settings will be local to this script only since it writes to
# $GMT_TMPDIR/gmt.conf
gmt set COLOR_MODEL rgb FONT_ANNOT_PRIMARY 14p

# Make grid file and color map in temporary directory
gmt grdmath -Rd -I1 Y = $GMT_TMPDIR/lat.nc
gmt makecpt -Crainbow -T-90/90 > $GMT_TMPDIR/lat.cpt

# The gmt grdimage command creates the history file $GMT_TMPDIR/gmt.history
gmt grdimage $GMT_TMPDIR/lat.nc -JK6.5i -C$GMT_TMPDIR/lat.cpt -P -K -nl > $ps
gmt pscoast -R -J -O -Dc -A5000 -Gwhite -Bx60g30 -By30g30 >> $ps

# Clean up all temporary files and the temporary directory
rm -rf $GMT_TMPDIR
