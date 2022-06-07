#!/usr/bin/env bash
#               GMT EXAMPLE 41
#
# Purpose:      Illustrate typesetting of legend with table
# GMT modules:  set, coast, legend, plot, makecpt
#
gmt begin ex41
	gmt set FONT_ANNOT_PRIMARY 12p,Helvetica FONT_LABEL 12p,Helvetica
	gmt makecpt -Cred,orange,yellow,green,bisque,cyan,magenta,white,gray -T1/10/1 -N
	gmt coast -R130W/50W/8N/56N -JM14c -Glightgray -Sazure1 -A1000 -Wfaint
	gmt coast -EUS+glightyellow+pfaint -ECU+glightred+pfaint -EMX+glightgreen+pfaint -ECA+glightblue+pfaint
	gmt coast -N1/1p,darkred -A1000/2/2 -Wfaint -Cazure1
	gmt plot -Sk@symbol_41/0.25c -C -W0.25p -: @data_41.txt -B0 --MAP_FRAME_TYPE=plain
	gmt legend -DJTL+w14c+jBL+l1.2+o0/0.3c -C4p -F+p+gsnow1 @table_41.txt
gmt end show
