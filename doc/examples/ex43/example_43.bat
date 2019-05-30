REM             GMT EXAMPLE 43
REM
REM Purpose:      Illustrate regression and outlier detection
REM GMT progs:    gmtregress, psbasemap, pstext, psxy
REM DOS calls:	  grep, paste, awk, sed
REM

echo GMT EXAMPLE 43
set ps=example_43.ps

gmt which -Gl @bb_weights.asc
gmt regress -Ey -Nw -i0-1l bb_weights.asc > model.txt
gmt regress -Ey -Nw -i0-1l bb_weights.asc -Fxmc -T-2/6/0.1 > rls_line.txt
gmt regress -Ey -N2 -i0-1l bb_weights.asc -Fxm -T-2/6/8 > ls_line.txt
grep -v '^>' model.txt > A.txt
grep -v '^#' bb_weights.asc > B.txt
gawk "{if ($7 == 0) printf "%dp\n", NR}" A.txt > sed.txt
gmt makecpt -Clightred,green -T0/2/1 -F+c -N > t.cpt
gmt psbasemap -R0.01/1e6/0.1/1e5 -JX6il -P -Ba1pf3 -Bx+l"Log@-10@- body weight (kg)" -By+l"Log@-10@- brain weight (g)" -BWSne+glightblue -K -X1.5i -Y4i > %ps%
gmt psxy -R-2/6/-1/5 -JX6i -O -K rls_line.txt -L+yt -Glightgoldenrod >> %ps%
sed -n -f sed.txt B.txt | gmt pstext -R0.01/1e6/0.1/1e5 -JX6il -O -K -F+f12p+jRM -Dj0.15i >> %ps%
gmt psxy -R-2/6/-1/5 -JX6i -O -K -L+d+p0.25p,- -Gcornsilk1 rls_line.txt >> %ps%
gmt psxy -R -J -O -K rls_line.txt -W3p >> %ps%
gmt psxy -R -J -O -K ls_line.txt -W1p,- >> %ps%
gmt psxy -R -J -O -K -Sc0.15i -Ct.cpt -Wfaint -i0,1,6 model.txt >> %ps%
gmt pstext A.txt -R -J -O -K -F+f8p+jCM+r1  -B0 >> %ps%
REM Build legend
echo H 18 Times-Roman Index of Animals > legend.txt
echo D 1p >> legend.txt
echo N 7 43 7 43 >> legend.txt
gawk -F'\t' "{printf "L - C %d.\nL - L %s\n", NR, $NF}" B.txt >> legend.txt
gmt pslegend -DjBR+w2.5i+o0.4c -R -J -O -K -F+p1p+gwhite+s+c3p+r legend.txt --FONT_LABEL=8p >> %ps%
gmt psbasemap -R0.5/28.5/-10/4 -JX6i/2i -O -K -Y-2.9i -B+glightgoldenrod >> %ps%
gmt psxy -R -J -O -K -Gcornsilk1 -W0.25p,- << EOF >> %ps%
>
0	-2.5
30	-2.5
30	2.5
0	2.5
> -Glightblue
0	-10
30	-10
30	-2.5
0	-2.5

EOF
awk '{print NR, $6, $7}' A | gmt psxy -R -J -O -K -Sb1ub0 -W0.25p -Ct.cpt >> %ps%
gmt psbasemap -R -J -O -Bafg100 -Bx+l"Index number" -By+l"z-zcore" -BWSne >> %ps%
del *.txt
del t.cpt
