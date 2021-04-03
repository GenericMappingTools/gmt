#!/usr/bin/env bash
# Make a figure illustrating transparency curve for an event label across time in psevents
gmt begin psevents_labels
	gmt set GMT_THEME cookbook
	cat <<- EOF > B.txt
	-0.5	afg	t@-r@-
	0	afg	t@-b@-
	3	afg	t@-e@-
	3.5	afg	t@-f@-
	EOF
	# Rise (t = -0.5 to 0 transparency goes from 100 to 0)
	gmt math -T-0.5/0/0.02 1 T 0.5 ADD 2 MUL SUB 2 POW 100 MUL = t.txt
	# normal (t = 0 to 3 transparency stays at 0)
	gmt math -T0/3/0.1 0 = >> t.txt
	# Fade (t = 3 to 3.5 transparency linearly increases to 100 during fading)
	gmt math -T3/3.5/0.1 T 3 SUB 2 MUL 100 MUL = >> t.txt
	# Code (t = 3.5 5 transparency stays at 100 during coda)
	gmt math -T3.5/5/0.1 100 = >> t.txt
	gmt basemap -R-1/5/-20/120 -JX6i/1.85i -BWStr -BxcB.txt -Bya+l"Label transparency"
	gmt plot -W0.25p,- <<- EOF
	-1	0
	6	0
	EOF
	gmt plot -W2p t.txt
	gmt plot -Sv0.15i+bt+et+s -W1.5p,blue <<- EOF
	0	110	3	110
	EOF
	gmt plot -St0.15i -Gblue <<- EOF
	0	-18
	3	-18
	EOF
	gmt text -F+f9p -Gwhite <<- EOF
	-0.25 -7 RISE
	1.5 -7 NORMAL
	3.25 -7 FADE
	4.25 -7 CODA
	1.5 110 FULL VISIBILITY OF LABEL
	EOF
	rm -f B.txt t.txt
gmt end show
