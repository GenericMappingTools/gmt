REM               GMT EXAMPLE 41
REM
REM Purpose:      Illustrate typesetting of legend with table
REM GMT modules:  set, coast, legend, plot, makecpt
REM
gmt begin ex41
	gmt set FONT_ANNOT_PRIMARY 12p FONT_LABEL 12p
	gmt makecpt -Cred,orange,yellow,green,bisque,cyan,magenta,white,gray -T1/10/1 -N
	gmt coast -R130W/50W/8N/56N -JM5.6i -B0 -Glightgray -Sazure1 -A1000 -Wfaint -Xc -Y1.2i --MAP_FRAME_TYPE=plain
	gmt coast -EUS+glightyellow+pfaint -ECU+glightred+pfaint -EMX+glightgreen+pfaint -ECA+glightblue+pfaint
	gmt coast -N1/1p,darkred -A1000/2/2 -Wfaint -Cazure1
	gmt plot -Sk@symbol_41/0.1i -C -W0.25p -: @data_41.txt
	gmt legend -R0/6/0/9.1 -Jx1i -Dx3i/4.5i+w5.6i+jBC+l1.2 -C0.05i -F+p+gsnow1 -B0 @table_41.txt -X-0.2i -Y-0.2i
gmt end show
