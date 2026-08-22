#!/usr/bin/env bash
#
# Testing all combinations of subplot behaviors with or without -S and how MAP_FRAME_AXES and -Bframes affect things
# subplot_1.x uses default settings and no -SR -SC
# subplot_2.x uses -SC
# subplot_3.x uses -SR
# subplot_4.x uses -SC -SR
# subplot_x.0 uses default settings
# subplot_x.1 sets MAP_FRAME_AXES via subplot begin
# subplot_x.2 adds -Bframes via subplot begin
# subplot_x.3 adds -Bframes just for one panel (LL)
# This results in 16 different 2x2 subplot examples

# 2.3 -SC, panel -B override
gmt begin subplot_2.3
	gmt set FONT_HEADING 28p,Helvetica,black
    gmt subplot begin 2x2 -Fs8c -SCb -R-2/2/0/10 -T"2.3: -SCb, LL panel -Bwsne"
       	gmt basemap -c
        gmt basemap -c
    	gmt basemap -c -Bwsne
      	gmt basemap -c
    gmt subplot end
gmt end show
