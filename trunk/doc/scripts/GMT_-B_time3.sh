#!/bin/bash
#	$Id: GMT_-B_time3.sh,v 1.9 2011-03-15 02:06:29 guru Exp $
#
. functions.sh
gmtset FORMAT_DATE_MAP o FORMAT_TIME_PRIMARY_MAP Character FONT_ANNOT_PRIMARY +9p
psbasemap -R1997T/1999T/0/1 -JX5i/0.2i -Bpa3Of1o -Bsa1YS -P > GMT_-B_time3.ps
