#!/usr/bin/env bash
#
# Problem is https://github.com/GenericMappingTools/gmt/issues/1759
# Can test with -+83 for a single square

ps=pscoast_JQ.ps

gmt pscoast -JQ200/6i -R-42.2839/223.375/-90/90 -BWeSn -Baf -Dc -S180/220/255 -G200 -Wfaint -P > $ps
