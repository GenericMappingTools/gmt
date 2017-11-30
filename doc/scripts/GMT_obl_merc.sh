#!/bin/bash
#	$Id$
#
gmt pscoast -R270/20/305/25r -JOc280/25.5/22/69/4.8i -Bag -Di -A250 -Gburlywood -Wthinnest -P \
	-TdjTR+w0.4i+f2+l+o0.15i -Sazure --FONT_TITLE=8p --MAP_TITLE_OFFSET=0.05i > GMT_obl_merc.ps
