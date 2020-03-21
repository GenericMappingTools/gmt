#!/usr/bin/env bash
# Make a figure illustrating transparency curve for an event across time in psevents
gmt begin psevents_transparency
	cat <<- EOF > B.txt
	-0.5	afg	t@-r@-
	0	afg	t@-b@-
	0.6	afg	t@-p@-
	1.5	afg	t@-d@-
	3	afg	t@-e@-
	3.5	afg	t@-f@-
	EOF
	# Rise (t = -0.5 to 0 transparency goes from 100 to 0)
	gmt math -T-0.5/0/0.02 1 T 0.5 ADD 2 MUL SUB 2 POW 100 MUL = t.txt
	# plateau, decay, normal (t = 0 to 3 transparency stays at 0)
	gmt math -T0/3/0.1 0 = >> t.txt
	# Fade (t = 3 to 3.5 transparency linearly increases to 75 during fading)
	gmt math -T3/3.5/0.1 T 3 SUB 2 MUL 75 MUL = >> t.txt
	# Code (t = 3.5 5 transparency stays at 75 during coda)
	gmt math -T3.5/5/0.1 75 = >> t.txt
	gmt basemap -R-1/5/-20/130 -JX6i/2i -BWStr -BxcB.txt -Bya+l"Symbol transparency"
	gmt plot -W0.25p,- <<- EOF
	-1	0
	6	0
	EOF
	gmt plot -W2p t.txt
	gmt plot -Sv0.15i+bt+et+s -W1.5p,red <<- EOF
	-0.5	110	1.5	110
	EOF
	gmt plot -Sv0.15i+bt+et+s -W1.5p,blue <<- EOF
	0	120	3	120
	EOF
	gmt plot -St0.15i -Gblue <<- EOF
	0	-18
	3	-18
	EOF
	gmt text -F+f9p -Gwhite <<- EOF
	-0.25 -7 RISE
	0.3 -7 PLATEAU
	1 -7 DECAY
	2.25 -7 NORMAL
	3.25 -7 FADE
	4.25 -7 CODA
	0.5 110 ANNOUNCE
	1.5 120 DURATION OF EVENT
	EOF
	rm -f B.txt t.txt
gmt end show

