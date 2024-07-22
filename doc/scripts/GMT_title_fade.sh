#!/usr/bin/env bash
gmt begin GMT_title_fade
	cat <<- EOF > B.txt
	0	afg	0
	1	afg	t@-i@-
	5	afg	t@-o@-
	6	afg	t@-b@-
	7	afg	f@-i@-
	14	afg	f@-o@-
	15	afg	t@-e@-
	EOF
	gmt math -T0/1/25+n 1 PI T MUL COS SUB 2 DIV 100 MUL = curve.txt
	gmt math -T1/5/1 100 = >> curve.txt
	gmt math -T5/6/25+n 1 PI T 5 SUB MUL COS ADD 2 DIV 100 MUL = >> curve.txt
	gmt math -T6/7/25+n 1 PI T 6 SUB MUL COS SUB 2 DIV 100 MUL = >> curve.txt
	gmt math -T7/14/1 100 = >> curve.txt
	gmt math -T14/15/25+n 1 PI T 14 SUB MUL COS ADD 2 DIV 100 MUL = >> curve.txt
	gmt plot -R0/15/0/140 -JX6i/1i -BxcB.txt -Byaf+l"fade-level" -W1.5p -BWS curve.txt
	gmt text -F+f10p+jCB <<- EOF
	3 120 TITLE SEQUENCE
	10.5 120 ANIMATION SEQUENCE
	EOF
	gmt plot -Sv24p+bt+et+s -W1.5p,red -N <<- EOF
	0	110	6	110
	6	110	15	110
	EOF
gmt end show
