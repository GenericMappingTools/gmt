REM               GMT EXAMPLE 43
REM
REM Purpose:      Illustrate regression and outlier detection
REM GMT modules:  gmtregress, basemap, legend, text, plot
REM DOS calls:	grep, awk, sed
REM

REM Data from Table 7 in Rousseeuw and Leroy, 1987.
gmt begin ex43

	gmt which -G @bb_weights.txt > file.txt
	set /p file=<file.txt
	gmt regress -Ey -Nw -i0:1+l %file% > model.txt
	gmt regress -Ey -Nw -i0:1+l %file% -Fxmc -T-2/6/0.1 > rls_line.txt
	gmt regress -Ey -N2 -i0:1+l %file% -Fxm -T-2/6/2+n > ls_line.txt
	grep -v "^>" model.txt > A.txt
	grep -v "^#" %file% > B.txt
	gawk "{if ($7 == 0) printf \"%%dp\n\", NR}" A.txt > sed.txt
	gmt makecpt -Clightred,green -T0/2/1 -F+c -N
	gmt basemap -R0.01/1e6/0.1/1e5 -JX15cl -Ba1pf3 -Bx+l"Log@-10@- body weight (kg)" -By+l"Log@-10@- brain weight (g)" -BWSne+glightblue -Y10c
	gmt plot -R-2/6/-1/5 -JX15c rls_line.txt -L+yt -Glightgoldenrod
	sed -n -f sed.txt B.txt | gmt text -R0.01/1e6/0.1/1e5 -JX15cl -F+f12p+jRM -Dj10p
	gmt plot -R-2/6/-1/5 -JX15c -L+d+p0.25p,- -Gcornsilk1 rls_line.txt
	gmt plot rls_line.txt -W3p
	gmt plot ls_line.txt -W1p,-
	gmt plot -Sc0.4c -C -Wfaint -i0,1,6 model.txt
	gmt text A.txt -F+f8p+jCM+r1 -B0
	REM Build legend
	echo H 18p,Times-Roman Index of Animals > legend.txt
	echo D 1p >> legend.txt
	echo N 7 43 7 43 >> legend.txt
	gawk -F"\t" "{printf \"L - C %%d.\nL - L %%s\n\", NR, $NF}" B.txt >> legend.txt
	gmt legend -DjBR+w2.5i+o0.4c -F+p1p+gwhite+s+c3p+r legend.txt --FONT_LABEL=8p
	gmt basemap -R0.5/28.5/-10/4 -J15c/5c -Y-7.5c -B+glightgoldenrod
	echo ^> > lines.txt
	echo 0	-2.5	>> lines.txt
	echo 30	-2.5	>> lines.txt
	echo 30	2.5		>> lines.txt
	echo 0	2.5		>> lines.txt
	echo ^> -Glightblue	>> lines.txt
	echo 0	-10		>> lines.txt
	echo 30	-10		>> lines.txt
	echo 30	-2.5	>> lines.txt
	echo 0	-2.5	>> lines.txt
	gmt plot lines.txt -Gcornsilk1 -W0.25p,-
	gawk "{print NR, $6, $7}" A.txt | gmt plot -Sb1ub0 -W0.25p -C
	gmt basemap -Bafg100 -Bx+l"Animal index number" -By+l"z-zcore" -BWSne
	del *.txt
gmt end show
