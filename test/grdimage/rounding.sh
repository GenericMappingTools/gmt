#!/usr/bin/env bash
# Failing script while fixing https://forum.generic-mapping-tools.org/t/error-pygmt-gmtcliberror-module-grdimage-failed-with-status-code-78/829
# Now reported as issue https://github.com/GenericMappingTools/gmt/pull/4130

gmt begin rounding ps
	gmt subplot begin 2x1 -Scb -Srl -Bwsne -Fs10c/0 -R3:57/4:18/44:00/44:15 -JM10c -X5c
		gmt grdimage -R3:57/4:18/44:00/44:15 -JM? @earth_relief_03s -Cgeo -I -c0
		gmt grdimage -R3.970/4.270/44.000/44.250 -JM? @earth_relief_03s -Cgeo -I -c1
	gmt subplot end
gmt end show
