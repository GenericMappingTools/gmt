#!/usr/bin/env bash
# Test gmt convert with different segment markers

ps=segments.ps

cat << EOF > gmt_i.txt
> Some ps=segments.ps
1	1
2	7
3	2
>Another ps=segments.ps
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
gmt convert gmt_i.txt --IO_SEGMENT_MARKER='>,B' > t.txt
gmt psxy -R0/10/0/10 -JX3i -P -K -Ba -BWSne gmt_i.txt -Sc0.2i -Ggreen -Y0.5i > $ps
gmt psxy -R -J -O -K t.txt -Sc0.1i -Gred --IO_SEGMENT_MARKER=B >> $ps
gmt psxy -R -J -O -K t.txt -W0.5p --IO_SEGMENT_MARKER=B >> $ps
echo "10 10 GMT->blank" | gmt pstext -R -J -O -K -F+jTR+f12 -Dj0.1i >> $ps
# Test GMT on input and NaN output
gmt convert gmt_i.txt --IO_SEGMENT_MARKER='>,N' > t.txt
gmt psxy -R -J -O -K -Ba -BWSne gmt_i.txt -Sc0.2i -Ggreen -X3.5i >> $ps
gmt psxy -R -J -O -K t.txt -Sc0.1i -Gred  --IO_SEGMENT_MARKER=N >> $ps
gmt psxy -R -J -O -K t.txt -W0.5p --IO_SEGMENT_MARKER=N >> $ps
echo "10 10 GMT->NaN" | gmt pstext -R -J -O -K -F+jTR+f12 -Dj0.1i >> $ps
# Test blank on input and GMT output
gmt convert blank_i.txt --IO_SEGMENT_MARKER='B,>' > t.txt
gmt psxy -R -J -O -K -Ba -BWSne blank_i.txt -Sc0.2i -Ggreen -X-3.5i -Y3.4i --IO_SEGMENT_MARKER=B >> $ps
gmt psxy -R -J -O -K t.txt -Sc0.1i -Gred >> $ps
gmt psxy -R -J -O -K t.txt -W0.5p >> $ps
echo "10 10 blank->GMT" | gmt pstext -R -J -O -K -F+jTR+f12 -Dj0.1i >> $ps
# Test blank on input and NaN output
gmt convert blank_i.txt --IO_SEGMENT_MARKER='B,N' > t.txt
gmt psxy -R -J -O -K -Ba -BWSne blank_i.txt -Sc0.2i -Ggreen -X3.5i  --IO_SEGMENT_MARKER=B >> $ps
gmt psxy -R -J -O -K t.txt -Sc0.1i -Gred --IO_SEGMENT_MARKER=N >> $ps
gmt psxy -R -J -O -K t.txt -W0.5p --IO_SEGMENT_MARKER=N >> $ps
echo "10 10 blank->NaN" | gmt pstext -R -J -O -K -F+jTR+f12 -Dj0.1i >> $ps
# Test NaN on input and GMT output
gmt convert nan_i.txt --IO_SEGMENT_MARKER='N,>' > t.txt
gmt psxy -R -J -O -K -Ba -BWSne nan_i.txt -Sc0.2i -Ggreen -X-3.5i -Y3.4i --IO_SEGMENT_MARKER=N >> $ps
gmt psxy -R -J -O -K t.txt -Sc0.1i -Gred >> $ps
gmt psxy -R -J -O -K t.txt -W0.5p >> $ps
echo "10 10 NaN->GMT" | gmt pstext -R -J -O -K -F+jTR+f12 -Dj0.1i >> $ps
# Test NaN on input and blank output
gmt convert nan_i.txt --IO_SEGMENT_MARKER='N,B' > t.txt
gmt psxy -R -J -O -K -Ba -BWSne nan_i.txt -Sc0.2i -Ggreen -X3.5i --IO_SEGMENT_MARKER=N >> $ps
gmt psxy -R -J -O -K t.txt -Sc0.1i -Gred  --IO_SEGMENT_MARKER=B >> $ps
gmt psxy -R -J -O -K t.txt -W0.5p  --IO_SEGMENT_MARKER=B >> $ps
echo "10 10 NaN->blank" | gmt pstext -R -J -O -F+jTR+f12 -Dj0.1i >> $ps
