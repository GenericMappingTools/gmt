#!/usr/bin/env bash
# Make a figure illustration intensity curve for an event across time in psevents
gmt begin psevents_intensity
	cat <<- EOF > B.txt
	-0.5	afg	t@-r@-
	0	afg	t@-b@-
	0.6	afg	t@-p@-
	1.5	afg	t@-d@-
	3	afg	t@-e@-
	3.5	afg	t@-f@-
	EOF
	# Rise (t = -0.5 to 0 intensity goes from 0 to 5)
	gmt math -T-0.5/0/0.02 T 0.5 ADD 2 MUL 2 POW 5 MUL = t.txt
	# plateau (t = 0 to 0.6 intensity stays at 5)
	gmt math -T0/0.6/0.1 5 = >> t.txt
	# Decay (t = 0.6 to 1.5 intensity decays from 5 to 0)
	gmt math -T0.6/1.5/0.02 0.9 T 0.6 SUB SUB 0.9 DIV 2 POW 5 MUL = >> t.txt
	# active (t = 1.5 to 3 intensity stays at 0)
	gmt math -T1.5/3/0.1 0 = >> t.txt
	# Fade (t = 3 to 3.5 intensity linearly drops to -0.75 during fading)
	gmt math -T3/3.5/0.1 T 3 SUB 1.5 MUL NEG = >> t.txt
	# Code (t = 3.5 5 intensity stays at -0.75 during coda)
	gmt math -T3.5/5/0.1 -0.75 = >> t.txt
	gmt basemap -R-1/5/-1/6.5 -JX6i/2i -BWStr -BxcB.txt -Bya+l"Color intensity"
	gmt plot -W0.25p,- <<- EOF
	-1	0
	6	0
	EOF
	gmt plot -W2p t.txt
	gmt plot -Sv0.15i+bt+et+s -W1.5p,red <<- EOF
	-0.5	5.5	1.5	5.5
	EOF
	gmt plot -Sv0.15i+bt+et+s -W1.5p,blue <<- EOF
	0	6	3	6
	EOF
	gmt plot -St0.15i -Gblue <<- EOF
	0	-0.9
	3	-0.9
	EOF
	gmt text -F+f9p -Gwhite <<- EOF
	-0.25 -0.4 RISE
	0.3 -0.4 PLATEAU
	1 -0.4 DECAY
	2.25 -0.4 NORMAL
	3.25 +0.4 FADE
	4.25 -0.4 CODA
	0.5 5.5 ANNOUNCE
	1.5 6 DURATION OF EVENT
	EOF
	rm -f B.txt t.txt
gmt end show

