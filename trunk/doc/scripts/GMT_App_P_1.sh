#!/bin/bash
#		GMT Appendix P, example 1
#		$Id: GMT_App_P_1.sh,v 1.5 2011-03-15 02:06:29 guru Exp $
#
# Purpose:	Illustrates the use of isolation mode
# GMT progs:	gmtset, grdimage, grdmath, makecpt, pscoast
# Unix progs:	mktemp, rm
#
. functions.sh
ps=GMT_App_P_1.ps

# Create a temporary directory. $GMT_TMPDIR will be set to its pathname.
# XXXXXX is replaced by a unique random combination of characters.
export GMT_TMPDIR=`mktemp -d /tmp/gmt.XXXXXX`

# These settings will be local to this script only since it writes to
# $GMT_TMPDIR/gmt.conf
gmtset COLOR_MODEL rgb FONT_ANNOT_PRIMARY 14p

# Make grid file and color map in temporary directory
grdmath -Rd -I1 Y = $GMT_TMPDIR/lat.nc
makecpt -Crainbow -T-90/90/180 -Z > $GMT_TMPDIR/lat.cpt

# The grdimage command creates the history file $GMT_TMPDIR/.gmtcommands4
grdimage $GMT_TMPDIR/lat.nc -Sl -JK6.5i -C$GMT_TMPDIR/lat.cpt -P -K > $ps
pscoast -R -J -O -Dc -A5000 -Gwhite -B60g30/30g30 >> $ps

# Clean up all temporary files and the temporary directory
rm -rf $GMT_TMPDIR
