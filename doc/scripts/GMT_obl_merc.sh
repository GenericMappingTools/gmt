#!/bin/bash
#	$Id: GMT_obl_merc.sh,v 1.10 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh

pscoast -R270/20/305/25r -JOc280/25.5/22/69/4.8i -B10g5 -Di -A250 -Glightgray -Wthinnest -P \
	-Tf301.5/23/0.4i/2 --FONT_TITLE=8p --MAP_TITLE_OFFSET=0.05i > GMT_obl_merc.ps
