#!/usr/bin/env bash
# Address issue https://github.com/GenericMappingTools/gmt/issues/4082
gmt math -T0/2pi/361+n T 2 MUL COS 0.9 MUL = t.txt
gmt begin snakes ps
	gmt subplot begin 4x1 -Fs17c/5c -M12p -A -R-pi6/13pi6/-1/1.5 -Scb -Srl -Bxapif -Byaf -Bwsrt
		gmt plot -W6p,sienna+v2.2c+gdarkgreen+h1 t.txt
		gmt plot -W6p,sienna+v2.2c+gdarkgreen+h1+p t.txt -c
		gmt plot -W6p,sienna+v2.2c+gdarkgreen+h1+p0.25p t.txt -c
		gmt plot -W6p,sienna+v2.2c+h1 t.txt -c
	gmt subplot end
gmt end show
