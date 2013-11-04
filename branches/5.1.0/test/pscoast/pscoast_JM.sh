#!/bin/bash
#
#	$Id$
# Make sure when fixed it works for all resolutions -D?

ps=pscoast_JM.ps

gmt pscoast -R90/290/-70/65 -JM6i -P -Ggray -Baf > $ps
