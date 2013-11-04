#!/bin/bash
#		GMT Appendix P, example 2
#		$Id$
#
# Purpose:	Illustrates the use of isolation mode
# GMT progs:	gmt gmtset, gmt grdimage, gmt grdmath, gmt makecpt, gmt pscoast
# GMT funcs:	gmt_init_tmpdir, gmt_remove_tmpdir
#
ps=GMT_App_P_2.ps

# Make GMT shell functions accessible the the script
. gmt_shell_functions.sh

# Create a temporary directory. $GMT_TMPDIR will be set to its pathname.
gmt_init_tmpdir

# These settings will be local to this script only since it writes to
# $GMT_TMPDIR/gmt.conf
gmt gmtset FONT_ANNOT_PRIMARY 14p

# Make grid file and color map in temporary directory
gmt grdmath -Rd -I1 Y = $GMT_TMPDIR/lat.nc
gmt makecpt -Crainbow -T-90/90/180 -Z > $GMT_TMPDIR/lat.cpt

# The gmt grdimage command creates the history file $GMT_TMPDIR/gmt.history
gmt grdimage $GMT_TMPDIR/lat.nc -JK6.5i -C$GMT_TMPDIR/lat.cpt -P -K -nl > $ps
gmt pscoast -R -J -O -Dc -A5000 -Gwhite -Bx60g30 -By30g30 >> $ps

# Clean up all temporary files and the temporary directory
gmt_remove_tmpdir
