#!/usr/bin/env bash
#
# Make sure when fixed it works for all resolutions -D?

ps=pscoast_JA.ps

gmt pscoast -JA13:25/52:31/10/7i -Rg -Gred -Sblue -Dl -P > $ps

