#!/usr/bin/env bash
# Test gmt psevents with lines

ps=lines.ps

# Create a sinusoid, then only plot what should be visible at t = 200 given a duration of 90
echo "> Snake line" > line.txt
gmt math -T0/360/1 T SIND -o0,1,0 = >> line.txt
gmt psevents -R-40/400/-2/2 -JX15c/10c -Baf -B+t"Snake line" line.txt -Ar -T200 -Es+r5+f5 -W2p,red -L90 -P > $ps
