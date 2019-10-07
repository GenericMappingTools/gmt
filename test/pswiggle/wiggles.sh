#!/usr/bin/env bash
# Test basic pswiggle operations
# Create an artificial data set of cosine along track
ps=wiggles.ps
cat << EOF | gmt sample1d -I5k | gmt mapproject -G+uk | gmt math STDIN -C2 10 DIV COS = line.txt
0	7
1	8
8	3
10	7
7.5	8
3.5	1.5
1.5	3.5
4.5	10
EOF
# Make the plots
gmt pswiggle -R-1/11/0/12 -JM3i -Baf -BWSne -P -K -W0.25p line.txt -Z4c -G+green -G-blue -T0.5p -A -Y0.75i -DjBR+w2+o0.2i > $ps
gmt pstext -R -J -O -K -F+f10p+jLB+cLB+t"-A" -Dj0.1i >> $ps
gmt pswiggle -R -J -W0.25p line.txt -Baf -BWSnE -Z4c -G+green -G-blue -T0.5p -O -K -X3.5i -DjBR+w2+o0.2i >> $ps
gmt pstext -R -J -O -K -F+f10p+jLB+cLB+t"[-A0]" -Dj0.1i >> $ps
gmt pswiggle -R -J -W0.25p line.txt -Baf -BWsne -Z4c -G+green -G-blue -T0.5p -A90 -O -K -X-3.5i -Y3.2i -DjBR+w2+o0.2i >> $ps
gmt pstext -R -J -O -K -F+f10p+jLB+cLB+t"-A90" -Dj0.1i >> $ps
gmt pswiggle -R -J -W0.25p line.txt -Baf -BWsnE -Z4c -G+green -G-blue -T0.5p -A225 -O -K -X3.5i -DjBR+w2+o0.2i >> $ps
gmt pstext -R -J -O -K -F+f10p+jLB+cLB+t"-A225" -Dj0.1i >> $ps
gmt pswiggle -R -J -W0.25p line.txt -Baf -BWsNe -Z4c -G+green -G-blue -T0.5p -I270 -O -K -X-3.5i -Y3.2i -DjBR+w2+o0.2i >> $ps
gmt pstext -R -J -O -K -F+f10p+jLB+cLB+t"-I270" -Dj0.1i >> $ps
gmt pswiggle -R -J -W0.25p line.txt -Baf -BWsNE -Z4c -G+green -G-blue -T0.5p -I5 -O -K -X3.5i -DjBR+w2+o0.2i  >> $ps
gmt pstext -R -J -O -F+f10p+jLB+cLB+t"-I5" -Dj0.1i >> $ps
