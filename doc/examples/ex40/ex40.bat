REM               GMT EXAMPLE 40
REM
REM Purpose:      Illustrate line simplification and area calculations
REM GMT modules:  basemap, text, plot, gmtsimplify, gmtspatial, subplot
REM

gmt begin ex40
	gmt spatial @GSHHS_h_Australia.txt -fg -Qk > centroid.txt
	gmt spatial @GSHHS_h_Australia.txt -fg -Qk | gawk "{printf \"Full area = %%.0f km@+2@+\n\", $3}" > area.txt

	gmt subplot begin 2x1 -Fs14c/12c -R112/154/-40/-10 -JM14c
		gmt subplot set 0
		gmt basemap -B20+lightgray -BWsne+g240/255/240
		gmt plot @GSHHS_h_Australia.txt -Wfaint -G240/240/255
		gmt plot @GSHHS_h_Australia.txt -Sc0.01c -Gred
		gmt simplify @GSHHS_h_Australia.txt -T100k > T100k.txt
		gmt spatial T100k.txt -fg -Qk | gawk "{printf \"Reduced area = %%.0f km@+2@+\n\", $3}" > area_T100k.txt
		gmt plot -W1p,blue T100k.txt
		gmt plot -Sx0.75c -W3p centroid.txt
		gmt text -Dj8p-F+cLT+jTL+f18p+t"T = 100 km"
		gmt text area.txt -F+f14p+cCM
		gmt text area_T100k.txt -F+f14p+cLB -Dj14p

		gmt subplot set 1
		gmt basemap -B20+lightgray -BWSne+g240/255/240
		gmt plot @GSHHS_h_Australia.txt -Wfaint -G240/240/255
		gmt plot @GSHHS_h_Australia.txt -Sc0.01c -Gred
		gmt simplify @GSHHS_h_Australia.txt -T500k > T500k.txt
		gmt spatial T500k.txt -fg -Qk | gawk "{printf \"Reduced area = %%.0f km@+2@+\n\", $3}" > area_T500k.txt
		gmt plot -W1p,blue T500k.txt
		gmt plot -Sx0.75c -W3p centroid.txt
		gmt text -Dj8p -F+cLT+jTL+f18p+t"T = 500 km"
		gmt text area.txt -F+f14p+cCM
		gmt text area_T500k.txt -F+f14p+cLB -Dj14p
	gmt subplot end
	del centroid.txt area*.txt T*.txt
gmt end show
