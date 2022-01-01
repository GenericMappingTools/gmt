#!/usr/bin/env bash
#
# Demonstrate the different line caps in PostScript
#
gmt begin GMT_cap ps
	gmt set GMT_THEME cookbook
	cat <<-EOF > butt.txt
	30	50
	170	50
	EOF
	cat <<-EOF > round.txt
	30	70
	170	70
	EOF
	cat <<-EOF > square.txt
	30	90
	170	90
	EOF
	# round
	gmt plot -R0/250/0/100 -Jx1p --PS_LINE_CAP=butt -W10p,lightred,20_20:0 butt.txt
	gmt plot -Wfaint butt.txt
	gmt plot -Sc3p -Gwhite -Wfaint butt.txt
	# miter
	gmt plot --PS_LINE_CAP=round -W10p,lightblue,,20_20:0 round.txt
	gmt plot -Wfaint round.txt
	gmt plot -Sc3p -Gwhite -Wfaint round.txt
	# bevel
	gmt plot --PS_LINE_CAP=square -W10p,lightorange,,20_20:0 square.txt
	gmt plot -Wfaint square.txt
	gmt plot -Sc3p -Gwhite -Wfaint square.txt
	gmt text -F+f8p,Helvetica-Bold+j -Dj5p <<- EOF
	180	50	ML	BUTT
	180	90	ML	SQUARE
	180	70	ML	ROUND
	EOF
gmt end show
