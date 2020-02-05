#!/usr/bin/env bash
# Test contour map with legends
gmt begin 4legends ps
    gmt subplot begin 2x2 -Fs3i -SRl -SCb -R-2/2/-2/2
       	echo 0 0 | gmt plot -Sc0.1i -Gblue -lThird -c0,1
        echo 0 0 | gmt plot -Sc0.1i -Gblack -lFourth -c1,1
    	echo 0 0 | gmt plot -Sc0.1i -Gred -lFirst -c0,0
    	gmt subplot set 1,0
      	echo 0 0 | gmt plot -Sc0.1i -Ggreen -lSecond
    gmt subplot end
gmt end show
