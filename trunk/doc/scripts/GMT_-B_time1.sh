#!/bin/sh
#	$Id: GMT_-B_time1.sh,v 1.1 2001-09-25 01:09:26 pwessel Exp $
#
gmtset PLOT_DATE_FORMAT -o
psbasemap -R2000-4-1T/2000-5-25T/0/1 -JX6T/0.2 -Bi7kIf1Of1dS -P > GMT_-B_time1.ps
