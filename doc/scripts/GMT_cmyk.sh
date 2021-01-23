#!/usr/bin/env bash
#
# Makes RGB and CMYK color circles
#

# Make various circles of radius 1.3 centered on different (x,y) origins
gmt math -T0/360/1 T -C0 COSD 1.3 MUL -0.75 ADD -C1 SIND 1.3 MUL 1.299038 ADD = R.d
gmt math -T0/360/1 T -C0 COSD 1.3 MUL +0.75 ADD -C1 SIND 1.3 MUL 1.299038 ADD = G.d
gmt math -T0/360/1 T -C0 COSD 1.3 MUL -C1 SIND 1.3 MUL = B.d
gmt math -T0/360/1 T -C0 COSD 1.3 MUL 4.25 ADD -C1 SIND 1.3 MUL 1.299038 ADD = M.d
gmt math -T0/360/1 T -C0 COSD 1.3 MUL 5.75 ADD -C1 SIND 1.3 MUL 1.299038 ADD = C.d
gmt math -T0/360/1 T -C0 COSD 1.3 MUL 5 ADD -C1 SIND 1.3 MUL = Y.d

gmt begin GMT_colors ps
	gmt plot -R-2.25/7.25/-1.8/2.75 -Jx0.6i R.d -Gred -B0
	gmt plot G.d -Ggreen
	gmt plot B.d -Gblue
	gmt plot C.d -Gcyan
	gmt plot M.d -Gmagenta
	gmt plot Y.d -Gyellow
	# Do R&G
	gmt convert R.d G.d | gmt clip -N
	gmt plot G.d -Gyellow
	gmt clip -C
	# Do R&B
	gmt convert R.d B.d | gmt clip -N
	gmt plot R.d -Gmagenta
	gmt clip -C
	# Do G&B
	gmt convert G.d B.d | gmt clip -N
	gmt plot G.d -Gcyan
	gmt clip -C
	# Do W
	gmt select -FR.d B.d | gmt select -FG.d > b.txt
	gmt select -FR.d G.d | gmt select -FB.d > g.txt
	gmt select -FB.d R.d | gmt select -FG.d > r.txt
	cat r.txt b.txt g.txt | gmt plot -Gwhite
	# Do C&M
	gmt convert C.d M.d | gmt clip -N
	gmt plot C.d -Gblue
	gmt clip -C
	# Do C&Y
	gmt convert C.d Y.d | gmt clip -N
	gmt plot C.d -Ggreen
	gmt clip -C
	# Do M&Y
	gmt convert M.d Y.d | gmt clip -N
	gmt plot M.d -Gred
	gmt clip -C
	# Do K
	gmt select -FC.d Y.d | gmt select -FM.d > y.txt
	gmt select -FC.d M.d | gmt select -FY.d > m.txt
	gmt select -FY.d C.d | gmt select -FM.d > c.txt
	cat c.txt y.txt m.txt | gmt plot -Gblack
	gmt plot [GRBCMY].d -W1p
	gmt text -F+f16p,Bookman-Demi+jBC <<- EOF
	0		-1.7	LIGHT
	5		-1.7	PAINT
	EOF
	gmt text -F+f18p,Bookman-Demi+jCM <<- EOF
	-1.5	1.299038	R
	1.5		1.299038	G
	0		-0.75		B
	3.5		1.299038	M
	6.5		1.299038	C
	5		-0.75		Y
	EOF
gmt end show
rm -f [GRBCMY].d [rgbcmy].txt
