#!/usr/bin/env bash
#
# Demonstrate the different line joints in PostScript
#
gmt begin GMT_joint ps
	gmt set GMT_THEME cookbook
	cat <<-EOF > round.txt
	1	1
	8	2
	7	1
	EOF
	cat <<-EOF > miter.txt
	3 1
	9 3
	1 2
	EOF
	cat <<-EOF > bevel.txt
	0.5	1
	3	3
	2	0.2
	EOF
	# round
	gmt plot -R0/11/0/4 -Jx1.5c --PS_LINE_JOIN=round -W10p,lightred round.txt
	gmt plot -Wfaint round.txt
	gmt plot -Sc3p -Gwhite -Wfaint round.txt
	# miter
	gmt plot --PS_LINE_JOIN=miter --PS_MITER_LIMIT=1 -W10p,lightblue miter.txt
	gmt plot -Wfaint miter.txt
	gmt plot -Sc3p -Gwhite -Wfaint miter.txt
	# bevel
	gmt plot --PS_LINE_JOIN=bevel -W10p,lightorange bevel.txt
	gmt plot -Wfaint bevel.txt
	gmt plot -Sc3p -Gwhite -Wfaint bevel.txt
	gmt text -F+f14p,Helvetica-Bold+j -Dj5p <<- EOF
	3	3	BR	BEVEL
	9	3	TL	MITER
	8	2	TL	ROUND
	EOF
gmt end show
