#!/bin/bash
# Test basic pswiggle operations
# Create an artificial data set of cosine along track
ps=wiggles.ps
cat << EOF | gmt sample1d -I5k | gmt mapproject -Gk | gmt math STDIN -C2 10 DIV COS = line.txt
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
gmt pswiggle -R-1/11/0/12 -JM3i -Baf -BWSne -P -K -W0.25p line.txt -Z4c -G+green -G-blue -T0.5p -A -Y0.75i -S8/1/2 > $ps
echo "-A" | gmt pstext -R -J -O -K -F+f10p+jLB+cLB -Dj0.1i >> $ps
gmt pswiggle -R -J -W0.25p line.txt -Baf -BWSnE -Z4c -G+green -G-blue -T0.5p -O -K -X3.5i -S8/1/2 >> $ps
echo "[-A0]" | gmt pstext -R -J -O -K -F+f10p+jLB+cLB -Dj0.1i >> $ps
gmt pswiggle -R -J -W0.25p line.txt -Baf -BWsne -Z4c -G+green -G-blue -T0.5p -A90 -O -K -X-3.5i -Y3.2i -S8/1/2 >> $ps
echo "-A90" | gmt pstext -R -J -O -K -F+f10p+jLB+cLB -Dj0.1i >> $ps
gmt pswiggle -R -J -W0.25p line.txt -Baf -BWsnE -Z4c -G+green -G-blue -T0.5p -A225 -O -K -X3.5i -S8/1/2 >> $ps
echo "-A225" | gmt pstext -R -J -O -K -F+f10p+jLB+cLB -Dj0.1i >> $ps
gmt pswiggle -R -J -W0.25p line.txt -Baf -BWsNe -Z4c -G+green -G-blue -T0.5p -I270 -O -K -X-3.5i -Y3.2i -S8/1/2 >> $ps
echo "-I270" | gmt pstext -R -J -O -K -F+f10p+jLB+cLB -Dj0.1i >> $ps
gmt pswiggle -R -J -W0.25p line.txt -Baf -BWsNE -Z4c -G+green -G-blue -T0.5p -I5 -O -K -X3.5i -S8/1/2  >> $ps
echo "-I5" | gmt pstext -R -J -O -F+f10p+jLB+cLB -Dj0.1i >> $ps
