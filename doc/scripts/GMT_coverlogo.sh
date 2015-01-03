#!/bin/bash
#	$Id$
#
# Creates the cover page GMT logo
#
#	Logo is 5.458" wide and 2.729" high and origin is lower left
#
gmt logo -W5.458i -P -X0 -Y0 > GMT_coverlogo.ps
