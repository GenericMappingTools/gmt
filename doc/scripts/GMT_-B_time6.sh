#!/bin/sh
#	$Id: GMT_-B_time6.sh,v 1.1 2001-09-25 01:09:26 pwessel Exp $
#
gmtset PLOT_DATE_FORMAT "o yy"
psbasemap -R1996T/1996-6T/0/1 -JX6T/0.2 -BiA1Of1dS -P > GMT_-B_time6.ps
