#!/usr/bin/env bash
# Test various ways of plotting vectors via psxyz

gmt begin vector3d_mods
	gmt set PROJ_LENGTH_UNIT cm
	scl=$(gmt math --FORMAT_FLOAT_OUT=%.3g -Q 2.54 INV =)
	scl1=$(gmt math --FORMAT_FLOAT_OUT=%.3g -Q 2.54 INV 10 DIV =)
	gmt set MAP_FRAME_TYPE=plain FORMAT_FLOAT_OUT=%5.2f
	# Get vector components of a 1 inch long vector with direction of 30 degrees from horizontal
	dx=$(gmt math -Q 2.54 30 COSD MUL =)
	dy=$(gmt math -Q 2.54 30 SIND MUL =)
	dx1=$(gmt math -Q $dx 10 MUL =)
	dy1=$(gmt math -Q $dy 10 MUL =)
	dx_km=$(gmt math -Q 100 30 COSD MUL =)
	dy_km=$(gmt math -Q 100 30 SIND MUL =)
	dx_n=$(gmt math -Q $dx_km 1.852 DIV =)
	dy_n=$(gmt math -Q $dy_km 1.852 DIV =)
	dx2=$(gmt math -Q $dx_km 0.1 MUL =)
	dy2=$(gmt math -Q $dy_km 0.1 MUL =)
	gmt makecpt -Cturbo -T0/40
	textopt="-F+f6.5p+jLT -Gwhite -Dj0.1c -N"
	### CARTESIAN
	# Direction, length given as 2.54, implying 2.54c
	echo 0 0 0 30 2.54 | gmt plot3d -R-0.1/3/-0.1/9 -Jx1i -Bafg -Sv20p+e -W2p -Gred
	echo "-0.1 0.9 Input: 30 2.54 Vector: -Sv20p+e" | gmt text $textopt
	# Direction, length given as 1, and change default unit to be inch
	echo 0 1 0 30 1 | gmt plot3d -Sv20p+e -W2p -Gred --PROJ_LENGTH_UNIT=inch
	echo "-0.1 1.9 Input: 30 1 Vector: -Sv20p+e --PROJ_LENGTH_UNIT=inch" | gmt text $textopt
	# Direction, length given as 1i, yet trying to change unit
	echo 0 2 0 30 1i | gmt plot3d -Sv20p+e -W2p -Gred --PROJ_LENGTH_UNIT=cm
	echo "-0.1 2.9 Input: 30 1i Vector: -Sv20p+e --PROJ_LENGTH_UNIT=cm" | gmt text $textopt
	# dx, dy in assumed plot units (cm) and unit scale
	echo 0 3 0 ${dx} ${dy} | gmt plot3d -Sv20p+e+z -W2p -Gred
	echo "-0.1 3.9 Input: ${dx} ${dy} Vector: -Sv20p+e+z" | gmt text $textopt
	# dx, dy in actual plot units (cm) and unit scale
	echo 0 4 0 ${dx}c ${dy}c | gmt plot3d -Sv20p+e+z -W2p -Gred
	echo "-0.1 4.9 Input: ${dx}c ${dy}c Vector: -Sv20p+e+z" | gmt text $textopt
	# dx, dy in specified user units with scale
	echo 0 5 0 ${dx} ${dy} | gmt plot3d -Sv20p+e+z1q -W2p -Gred
	echo "-0.1 5.9 Input: ${dx} ${dy} Vector: -Sv20p+e+z1q" | gmt text $textopt
	# dx, dy in specified plot units with scale, and color based on magintude
	echo 0 6 0 ${dx} ${dy} | gmt plot3d -Sv20p+e+z${scl}q+c -W2p -C --PROJ_LENGTH_UNIT=inch
	echo "-0.1 6.9 Input: ${dx} ${dy} Vector: -Sv20p+e+z${scl}q+c --PROJ_LENGTH_UNIT=inch" | gmt text $textopt
	# dx, dy in specified data units with scale, and color based on magintude
	echo 0 7 0 ${dx1} ${dy1} | gmt plot3d -Sv20p+e+z${scl1}q+c -W2p -C --PROJ_LENGTH_UNIT=inch
	echo "-0.1 7.9 Input: ${dx1} ${dy1} Vector: -Sv20p+e+z${scl1}q+c --PROJ_LENGTH_UNIT=inch" | gmt text $textopt
	# magnitude in specified data units with scale, and color based on magintude
	echo 0 8 0 30 25.4 | gmt plot3d -Sv20p+e+v${scl1}q+c -W2p+c -C --PROJ_LENGTH_UNIT=inch
	echo "-0.1 8.9 Input: 30 25.4 Vector: -Sv20p+e+v${scl1}q+c --PROJ_LENGTH_UNIT=inch" | gmt text $textopt
	### GEOVECTOR
	# Direction, length given as 100, imnplying 100k
	echo 0 0 0 60 100 | gmt plot3d -JM9.1i+dh -Bafg -S=20p+e -W2p -Gred -X3.5i
	echo "-0.1 0.9 Input: 60 100 Vector: -S=20p+e" | gmt text $textopt
	# Direction, length given as 54n ~ 100k
	echo 0 1 0 60 54n | gmt plot3d -S=20p+e -W2p -Gred
	echo "-0.1 1.9 Input: 60 54n Vector: -S=20p+e" | gmt text $textopt
	# Direction, length given as 100kÏ
	echo 0 2 0 60 100k | gmt plot3d -S=20p+e -W2p -Gred
	echo "-0.1 2.9 Input: 60 100k Vector: -S=20p+e" | gmt text $textopt
	# dx_km, dy_km to mimick the same
	echo 0 3 0 ${dx_km} ${dy_km} | gmt plot3d -S=20p+e+z -W2p -Gred
	echo "-0.1 3.9 Input: ${dx_km} ${dy_km} Vector: -S=20p+e+z" | gmt text $textopt
	# dx_km, dy_km to mimick the same but with unit
	echo 0 4 0 ${dx_km}k ${dy_km}k | gmt plot3d -S=20p+e+z -W2p -Gred
	echo "-0.1 4.9 Input: ${dx_km}k ${dy_km}k Vector: -S=20p+e+z" | gmt text $textopt
	# dx_km, dy_km to mimick the same
	echo 0 5 0 ${dx_n}n ${dy_n}n | gmt plot3d -S=20p+e+z -W2p -Gred
	echo "-0.1 5.9 Input: ${dx_n}n ${dy_n}n Vector: -S=20p+e+z" | gmt text $textopt
	# dx, dy in specific data units with scale
	echo 0 6 0 ${dx2} ${dy2} | gmt plot3d -S=20p+e+z10q -W2p -Gred
	echo "-0.1 6.9 Input: ${dx2} ${dy2} Vector: -S=20p+e+z10q" | gmt text $textopt
	# dx, dy in specific data units with scale, and color based on magintude
	echo 0 7 0 ${dx2} ${dy2} | gmt plot3d -S=20p+e+z10q+c -W2p -C
	echo "-0.1 7.9 Input: ${dx2} ${dy2} Vector: -S=20p+e+z10q+c" | gmt text $textopt
	# magnitude in specific data units with scale, and color based on magintude
	echo 0 8 0 60 10 | gmt plot3d -S=20p+e+v10q+c -W2p+c -C
	echo "-0.1 8.9 Input: 60 10 Vector: -S=20p+e+v10q+c" | gmt text $textopt
gmt end show
