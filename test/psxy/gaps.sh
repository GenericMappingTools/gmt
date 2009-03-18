#!/bin/sh
# Test sample1d interpolation with NaNs
. ../functions.sh
header "Test psxy with NaNs indicating line gaps"
ps=gaps.ps

cat << EOF > $$.txt
0	0
1	1
2	1
3	1
4	NaN
5	NaN
6	-1
7	0
8	1
9	2
10	NaN
11	0
12	0
13	2
14	1
EOF
grep -v NaN $$.txt | psxy -R-1/15/-3/3 -JX6i/3i -Sc0.1i -W0.25p -Ggreen -P -K -Y6i > $ps
awk '{if ($2 == "NaN") {print $1, -3}}' $$.txt | psxy -R -J -O -K -St0.2i -Gblack -W0.25p >> $ps
sample1d $$.txt -I0.1 -Fl | psxy -R -J -O -K -W0.25p,. >> $ps
sample1d $$.txt -I0.1 -Fc | psxy -R -J -O -K -W0.25p,- >> $ps
sample1d $$.txt -I0.1 -Fa | psxy -R -J -O -K -W0.25p -B5f1g1:".Skipping NaNs and interpolating through": --HEADER_FONT_SIZE=18 >> $ps
# New behavior with upper case switches
grep -v NaN $$.txt | psxy -R -J -Sc0.1i -W0.25p -Ggreen -O -K -Y-4.5i >> $ps
awk '{if ($2 == "NaN") {print $1, -3}}' $$.txt | psxy -R -J -O -K -St0.2i -Gblack -W0.25p >> $ps
sample1d $$.txt -I0.1 -FL | psxy -R -J -O -K -W0.25p,. -F >> $ps
sample1d $$.txt -I0.1 -FC | psxy -R -J -O -K -W0.25p,- -F >> $ps
sample1d $$.txt -I0.1 -FA | psxy -R -J -O -K -W0.25p -F -B5f1g1:".Honoring NaNs as segment indicators": --HEADER_FONT_SIZE=18  >> $ps
echo "0 -4.5 16 0 0 LB Black triangles indicate NaN locations" | pstext -R -J -O -K -N >> $ps

psxy -R -J -O /dev/null >> $ps
rm -f $$.txt

pscmp
