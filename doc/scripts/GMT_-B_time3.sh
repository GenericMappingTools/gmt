#!/bin/sh
#	$Id: GMT_-B_time3.sh,v 1.1 2001-09-25 01:09:26 pwessel Exp $
#
gmtset PLOT_DATE_FORMAT o
psbasemap -R1997T/1999T/0/1 -JX6T/0.2 -Bic3Of1oI1YS -P > GMT_-B_time3.ps
