#!/usr/bin/env bash
# Basic test of variable filter widths
# Make a fake data set
gmt math -T0/1000/1 T 0.5 MUL COSD 4 MUL T 2 MUL COSD ADD T 20 MUL SIND 0.5 MUL ADD = data.txt
# Make a periodic set of filter widths from 5 to 115
gmt math -T0/1000/1 T SIND 55 MUL 60 ADD = fw.txt
gmt begin varfilt ps
	gmt subplot begin 2x1 -Fs17c/10c -Srl -Scb -A+jTR -R0/1000/-6/6 -M12p -Bwsrt -T"Variable Filter Widths"
		gmt subplot set 0 -AGaussian
		gmt filter1d data.txt -Fgfw.txt -T0/1000/1 > out.txt
		gmt plot -W0.25p data.txt
		gmt plot -W0.5p,orange out.txt
		gmt subplot set 1 -AMedian
		gmt filter1d data.txt -Fmfw.txt -T0/1000/1 > out.txt
		gmt plot -W0.25p data.txt
		gmt plot -W0.5p,orange out.txt
	gmt subplot end
gmt end show
