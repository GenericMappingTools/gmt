#!/usr/bin/env bash
#
# Check plotting of boxes in stereographic polar gmt projection
# Original script by Bruce Raup.

ps=polar.ps

gmt pscoast -R315/20/135/20r -JS0/90/15c -Dl -B45g45 -BWESN -A15000 -W0.25p -Slightblue -Gyellow -P -K > $ps

# Pan-Arctic domain
gmt psxy -R -J -W3p,blue -A -L -O -K << END >> $ps
-222.075 34.3318
-137.925 34.3318
-43.708 35.7708
43.7078 35.7708
-222.075 34.3318
END

# Hi-Res domain 1
gmt psxy -R -J -W2p -A -L -O -K << END >> $ps
241.821 70.3805
189.971 52.1708
141.2   55.1768
83.6929 78.1192
END

# Hi-Res domain 2
gmt psxy -R -J -W2p,100 -A -L -O -K << END >> $ps
344.055 71.727
147.002 84.1946
86.0915 69.7863
26.9868 63.5625
END

gmt pstext -R -J -F+fHelvetica-Bold+f -O -M -N << END >> $ps
# This is an optional ps=polar.ps
> 170 70 15p 16p 5c c
High Resolution Domain One
>  47 78 15p 16p 3c c
@;100;High Resolution Domain Two@;;
> 335 55 19p 20p 5c c
@;blue;Pan-Arctic Domain@;;
END

