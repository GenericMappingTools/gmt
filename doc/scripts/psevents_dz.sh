#!/usr/bin/env bash
# Make a figure illustrating dz curve for an event across time in psevents
gmt begin psevents_dz
	gmt set GMT_THEME cookbook
	cat <<- EOF > B.txt
	-0.5	afg	t@-r@-
	0	afg	t@-b@-
	0.6	afg	t@-p@-
	1.5	afg	t@-d@-
	3	afg	t@-e@-
	3.5	afg	t@-f@-
	EOF
	# Rise (t = -0.5 to 0 symbol dz goes from 0 to 1)
	gmt math -T-0.5/0/0.02 T 0.5 ADD 2 MUL 2 POW = t.txt
	# plateau (t = 0 to 0.5 symbol dz stays at 1)
	gmt math -T0/0.6/0.1 1 = >> t.txt
	# Decay (t = 0.5 to 1.5 symbol dz decays from 1 to 0)
	gmt math -T0.6/1.5/0.02 0.9 T 0.6 SUB SUB 0.9 DIV 2 POW = >> t.txt
	# active (t = 1.5 to 3 symbol dz stays at 0)
	gmt math -T1.5/3/0.1 0 = >> t.txt
	# Fade (t = 3 to 3.5 symbol dz linearly drops to -0.2 during fading)
	gmt math -T3/3.5/0.1 T 3 SUB 0.4 MUL NEG = >> t.txt
	# Coda (t = 3.5 5 symbol dz stays at -0.2 during code)
	gmt math -T3.5/5/0.1 -0.2 = >> t.txt
	gmt basemap -R-1/5/-0.30/1.30 -JX6i/2i -BWStr -BxcB.txt -Bya+l"Symbol @~D@~z"
	gmt plot -W0.25p,- <<- EOF
	-1	0
	6	0
	EOF
	gmt plot -W2p t.txt
	gmt plot -Sv0.15i+bt+et+s -W1.5p,red <<- EOF
	-0.5	1.10	1.5	1.10
	EOF
	gmt plot -Sv0.15i+bt+et+s -W1.5p,blue <<- EOF
	0	1.20	3	1.20
	EOF
	gmt plot -St0.15i -Gblue <<- EOF
	0	-0.18
	3	-0.18
	EOF
	gmt text -F+f9p -Gwhite <<- EOF
	-0.25 -0.07 RISE
	0.3 -0.07 PLATEAU
	1 -0.07 DECAY
	2.25 -0.07 NORMAL
	3.25 -0.07 FADE
	4.25 -0.07 CODA
	0.5 1.10 ANNOUNCE
	1.5 1.20 DURATION OF EVENT
	EOF
	rm -f B.txt t.txt
gmt end show
