#!/bin/sh
#	$Id: GMT_obl_merc.sh,v 1.6 2006-10-24 01:53:19 remko Exp $
#

pscoast -R270/20/305/25r -JOc280/25.5/22/69/4.8i -B10g5 -Di -A250 -Glightgray -Wthinnest -P \
	-Tf301.5/23/0.4i/2 --HEADER_FONT_SIZE=8p --LABEL_OFFSET=0.05i > GMT_obl_merc.ps
