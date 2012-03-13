#!/bin/bash
# Test gmtconvert with different segment markers

. ./functions.sh
header "Let gmtconvert handle different segment markers"

cat << EOF > gmt_i.txt
> Some header
1	1
2	7
3	2
>Another header
6	3
8	4
9	3
EOF
cat << EOF > blank_i.txt

1	1
2	7
3	2

6	3
8	4
9	3
EOF
cat << EOF > nan_i.txt
NaN	NaN
1	1
2	7
3	2
NaN	NaN
6	3
8	4
9	3
EOF
# Test GMT on input and blank output
gmtconvert gmt_i.txt --IO_SEGMENT_MARKER='>,B' > t.txt
psxy -R0/10/0/10 -JX3i -P -K -BaWSne gmt_i.txt -Sc0.2i -Ggreen -Y0.5i > $ps
psxy -R -J -O -K t.txt -Sc0.1i -Gred --IO_SEGMENT_MARKER=B >> $ps
psxy -R -J -O -K t.txt -W0.5p --IO_SEGMENT_MARKER=B >> $ps
echo "10 10 GMT->blank" | pstext -R -J -O -K -F+jTR+f12 -Dj0.1i >> $ps
# Test GMT on input and NaN output
gmtconvert gmt_i.txt --IO_SEGMENT_MARKER='>,N' > t.txt
psxy -R -J -O -K -BaWSne gmt_i.txt -Sc0.2i -Ggreen -X3.5i >> $ps
psxy -R -J -O -K t.txt -Sc0.1i -Gred  --IO_SEGMENT_MARKER=N >> $ps
psxy -R -J -O -K t.txt -W0.5p --IO_SEGMENT_MARKER=N >> $ps
echo "10 10 GMT->NaN" | pstext -R -J -O -K -F+jTR+f12 -Dj0.1i >> $ps
# Test blank on input and GMT output
gmtconvert blank_i.txt --IO_SEGMENT_MARKER='B,>' > t.txt
psxy -R -J -O -K -BaWSne blank_i.txt -Sc0.2i -Ggreen -X-3.5i -Y3.4i --IO_SEGMENT_MARKER=B >> $ps
psxy -R -J -O -K t.txt -Sc0.1i -Gred >> $ps
psxy -R -J -O -K t.txt -W0.5p >> $ps
echo "10 10 blank->GMT" | pstext -R -J -O -K -F+jTR+f12 -Dj0.1i >> $ps
# Test blank on input and NaN output
gmtconvert blank_i.txt --IO_SEGMENT_MARKER='B,N' > t.txt
psxy -R -J -O -K -BaWSne blank_i.txt -Sc0.2i -Ggreen -X3.5i  --IO_SEGMENT_MARKER=B >> $ps
psxy -R -J -O -K t.txt -Sc0.1i -Gred --IO_SEGMENT_MARKER=N >> $ps
psxy -R -J -O -K t.txt -W0.5p --IO_SEGMENT_MARKER=N >> $ps
echo "10 10 blank->NaN" | pstext -R -J -O -K -F+jTR+f12 -Dj0.1i >> $ps
# Test NaN on input and GMT output
gmtconvert nan_i.txt --IO_SEGMENT_MARKER='N,>' > t.txt
psxy -R -J -O -K -BaWSne nan_i.txt -Sc0.2i -Ggreen -X-3.5i -Y3.4i --IO_SEGMENT_MARKER=N >> $ps
psxy -R -J -O -K t.txt -Sc0.1i -Gred >> $ps
psxy -R -J -O -K t.txt -W0.5p >> $ps
echo "10 10 NaN->GMT" | pstext -R -J -O -K -F+jTR+f12 -Dj0.1i >> $ps
# Test NaN on input and blank output
gmtconvert nan_i.txt --IO_SEGMENT_MARKER='N,B' > t.txt
psxy -R -J -O -K -BaWSne nan_i.txt -Sc0.2i -Ggreen -X3.5i --IO_SEGMENT_MARKER=N >> $ps
psxy -R -J -O -K t.txt -Sc0.1i -Gred  --IO_SEGMENT_MARKER=B >> $ps
psxy -R -J -O -K t.txt -W0.5p  --IO_SEGMENT_MARKER=B >> $ps
echo "10 10 NaN->blank" | pstext -R -J -O -K -F+jTR+f12 -Dj0.1i >> $ps
psxy -R -J -O -T >> $ps

pscmp
