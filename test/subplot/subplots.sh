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

gmt begin
	gmt set FONT_HEADING 28p,Helvetica,black

# 1.0 No -S no override
	gmt figure subplot_1.0
    gmt subplot begin 2x2 -Fs8c -R-2/2/0/10 -T"1.0: No -S"
       	gmt basemap -c
        gmt basemap -c
    	gmt basemap -c
      	gmt basemap -c
    gmt subplot end

# 1.1 No -S, MAP_FRAME_AXES override
	gmt figure subplot_1.1
    gmt subplot begin 2x2 -Fs8c -R-2/2/0/10 --MAP_FRAME_AXES=WSne -T"1.1: No -S, MAP_FRAME_AXES=WSne"
       	gmt basemap -c
        gmt basemap -c
    	gmt basemap -c
      	gmt basemap -c
    gmt subplot end

# 1.2 No -S, -B override
	gmt figure subplot_1.2
    gmt subplot begin 2x2 -Fs8c -R-2/2/0/10 -BWSne -T"1.2: No -S, -BWSne override"
       	gmt basemap -c
        gmt basemap -c
    	gmt basemap -c
      	gmt basemap -c
    gmt subplot end

# 1.3 No -S, panel -B override
	gmt figure subplot_1.3
    gmt subplot begin 2x2 -Fs8c -R-2/2/0/10 -T"1.3: No -S, LL panel -BWSne"
       	gmt basemap -c
        gmt basemap -c
    	gmt basemap -c -BWSne
      	gmt basemap -c
    gmt subplot end

# Use -SC
# 2.0 -SC no override
	gmt figure subplot_2.0
    gmt subplot begin 2x2 -Fs8c -SCb -R-2/2/0/10 -T"2.0: -SCb"
       	gmt basemap -c
        gmt basemap -c
    	gmt basemap -c
      	gmt basemap -c
    gmt subplot end

# 2.1 -SC, MAP_FRAME_AXES override
	gmt figure subplot_2.1
    gmt subplot begin 2x2 -Fs8c -SCb -R-2/2/0/10 --MAP_FRAME_AXES=WSne -T"2.1: -SCb, MAP_FRAME_AXES=WSne"
       	gmt basemap -c
        gmt basemap -c
    	gmt basemap -c
      	gmt basemap -c
    gmt subplot end

# 2.2 -SC, -B override
	gmt figure subplot_2.2
    gmt subplot begin 2x2 -Fs8c -SCb -R-2/2/0/10 -BWSne -T"2.2: -SCb, -BWSne override"
       	gmt basemap -c
        gmt basemap -c
    	gmt basemap -c
      	gmt basemap -c
    gmt subplot end

# 2.3 -SC, panel -B override
	gmt figure subplot_2.3
    gmt subplot begin 2x2 -Fs8c -SCb -R-2/2/0/10 -T"2.3: -SCb, LL panel -BWSne"
       	gmt basemap -c
        gmt basemap -c
    	gmt basemap -c -BWSne
      	gmt basemap -c
    gmt subplot end

# Use -SR
# 3.0 -SR no override
	gmt figure subplot_3.0
    gmt subplot begin 2x2 -Fs8c -SRl -R-2/2/0/10 -T"3.0: -SRl"
       	gmt basemap -c
        gmt basemap -c
    	gmt basemap -c
      	gmt basemap -c
    gmt subplot end

# 3.1 -SR, MAP_FRAME_AXES override 
	gmt figure subplot_3.1
    gmt subplot begin 2x2 -Fs8c -SRl -R-2/2/0/10 --MAP_FRAME_AXES=WSne -T"3.1: -SRl, MAP_FRAME_AXES=WSne"
       	gmt basemap -c
        gmt basemap -c
    	gmt basemap -c
      	gmt basemap -c
    gmt subplot end

# 3.2 -SR, -B override
	gmt figure subplot_3.2
    gmt subplot begin 2x2 -Fs8c -SRl -R-2/2/0/10 -BWSne -T"3.2: -SRl, -BWSne override"
       	gmt basemap -c
        gmt basemap -c
    	gmt basemap -c
      	gmt basemap -c
    gmt subplot end

# 3.3 -SR, panel -B override
	gmt figure subplot_3.3
    gmt subplot begin 2x2 -Fs8c -SRl -R-2/2/0/10 -T"3.3: -SRl, LL panel -BWSne"
       	gmt basemap -c
        gmt basemap -c
    	gmt basemap -c -BWSne
      	gmt basemap -c
    gmt subplot end

# Use -SR and SC
# 4.0 -SR -SC no override
	gmt figure subplot_4.0
    gmt subplot begin 2x2 -Fs8c -SCb -SRl -R-2/2/0/10 -T"4.0: -SRl -SCb"
       	gmt basemap -c
        gmt basemap -c
    	gmt basemap -c
      	gmt basemap -c
    gmt subplot end

# 4.1 -SR -SC, MAP_FRAME_AXES override 
	gmt figure subplot_4.1
    gmt subplot begin 2x2 -Fs8c -SCb -SRl -R-2/2/0/10 --MAP_FRAME_AXES=WESN -T"4.1: -SRl -SCb, MAP_FRAME_AXES=WESN"
       	gmt basemap -c
        gmt basemap -c
    	gmt basemap -c
      	gmt basemap -c
    gmt subplot end

# 4.2 -SR -SC, -B override
	gmt figure subplot_4.2
    gmt subplot begin 2x2 -Fs8c -SCb -SRl -R-2/2/0/10 -BWESN -T"4.2: -SRl -SCb, -BWESN override"
       	gmt basemap -c
        gmt basemap -c
    	gmt basemap -c
      	gmt basemap -c
    gmt subplot end

# 4.3 -SR -SC, panel -B override
	gmt figure subplot_4.3
    gmt subplot begin 2x2 -Fs8c -SCb -SRl -R-2/2/0/10 -T"4.3: -SRl -SCb, LL panel -BWESN"
       	gmt basemap -c
        gmt basemap -c
    	gmt basemap -c -BWESN
      	gmt basemap -c
    gmt subplot end

gmt end show
