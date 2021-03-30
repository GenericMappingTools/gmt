#!/usr/bin/env bash
#
# Creates the cover page GMT logo
#
#	Logo is 5.458" wide and 2.729" high and origin is lower left
#
gmt logo -Dx0/0+w5.458i -X0 -Y0 --GMT_THEME=cookbook -ps GMT_coverlogo
