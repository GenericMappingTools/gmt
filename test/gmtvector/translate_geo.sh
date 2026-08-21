#!/usr/bin/env bash
# Test the gmt vector -Tt option for fixed and variable geographic translation

ps=translate_geo.ps

# Make lon lat points that spiral out
gmt math -T0/360/15 -N5 -o1:4 T 36 DIV T -C1 COSD -C2 SIND -C3-4 0.1 MUL -Ca MUL = spiral.txt
gmt psxy -R-8/15/-10/6 -JM16c -Baf -BWSne -Sc0.3c -Ggreen spiral.txt -P -K -Xc > $ps
gmt vector spiral.txt -Tt60/200k -fg > shifted.txt
gmt psxy -R -J -O -K -Sc0.2c -Gred shifted.txt >> $ps
echo "Fixed translation" | gmt pstext -R -J -O -K -F+f16p+jTR+cTR -Dj0.5c -W1p >> $ps
gmt convert spiral.txt shifted.txt -A -o0,1,4,5 | gmt psxy -R -J -O -K -Sv0.1+s -W0.25p >> $ps
gmt psxy -R -J -Baf -BWSne -Sc0.3c -Ggreen spiral.txt -O -K -Y12.5c >> $ps
gmt vector spiral.txt -Ttk -fg > shifted.txt
gmt psxy -R -J -O -K -Sc0.2c -Gblue shifted.txt >> $ps
gmt convert spiral.txt shifted.txt -A -o0,1,4,5 | gmt psxy -R -J -O -K -Sv0.1+s -W0.25p >> $ps
echo "Variable translation" | gmt pstext -R -J -O -F+f16p+jTR+cTR -Dj0.5c -W1p >> $ps
