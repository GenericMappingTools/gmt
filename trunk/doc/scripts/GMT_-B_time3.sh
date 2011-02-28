#!/bin/bash
#	$Id: GMT_-B_time3.sh,v 1.8 2011-02-28 00:58:00 remko Exp $
#
. functions.sh
gmtset PLOT_DATE_FORMAT o TIME_FORMAT_PRIMARY Character ANNOT_FONT_SIZE_PRIMARY +9p
psbasemap -R1997T/1999T/0/1 -JX5/0.2 -Bpa3Of1o -Bsa1YS -P > GMT_-B_time3.ps
