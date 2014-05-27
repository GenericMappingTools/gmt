#!/bin/bash
#
#	$Id$
# Test a non-global -JE plot

ps=pscoast_JE3.ps

gmt pscoast -B10g10 -Je-70/-90/1:15000000 -R-95/-60/-75/-55 -Di -Ggreen -Sblue -P > $ps
