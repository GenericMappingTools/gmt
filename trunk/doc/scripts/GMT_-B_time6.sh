#!/bin/sh
#	$Id: GMT_-B_time6.sh,v 1.3 2004-04-12 21:41:47 pwessel Exp $
#
gmtset PLOT_DATE_FORMAT "o yy" TIME_FORMAT_PRIMARY Abbreviated
psbasemap -R1996T/1996-6T/0/1 -JX6T/0.2 -Ba1Of1dS -P > GMT_-B_time6.ps
