#!/bin/bash
#	$Id: GMT_-B_time6.sh,v 1.6 2011-02-28 00:58:01 remko Exp $
#
. functions.sh
gmtset PLOT_DATE_FORMAT "o yy" TIME_FORMAT_PRIMARY Abbreviated
psbasemap -R1996T/1996-6T/0/1 -JX5/0.2 -Ba1Of1dS -P > GMT_-B_time6.ps
