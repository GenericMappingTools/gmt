#!/bin/bash
#
#	$Id$

ps=position1.ps

font=Helvetica

gmtset FONT_ANNOT_PRIMARY 24p,$font

psbasemap -JX8i/5i -R0/12/0/12 -B1g1/1g1SW -K > $ps
pstext -F+j -R -J -O -K >> $ps <<%
 1 1 BR 1
 1 2 BC 1
 1 3 BL 1
 2 1 BR 2
 2 2 BC 2
 2 3 BL 2
 3 1 BR 3
 3 2 BC 3
 3 3 BL 3
 4 1 BR BR
 4 2 BC BC
 4 3 BL BL
 6 1 BR 1A
 6 2 BC 1A
 6 3 BL 1A
 7 1 BR 2A
 7 2 BC 2A
 7 3 BL 2A
 8 1 BR Z1
 8 2 BC Z1
 8 3 BL Z1
 9 1 BR 9
 9 2 BC 9
 9 3 BL 9
10 1 BR 10
10 2 BC 10
10 3 BL 10
11 1 BR 11
11 2 BC 11
11 3 BL 11
 1 5 BL 01111111111
 1 6 BL 11111111110
 1 7 BL 01234567890
6.5 5 BC 01111111111
6.5 6 BC 11111111110
6.5 7 BC 01234567890
12 5 BR 01111111111
12 6 BR 11111111110
12 7 BR 01234567890
%
pstext -F+j -Ggreen -R -J -O -K >> $ps <<%
 1 9 BR 10@+-1@+
 5 9 MR 10@+-1@+
 9 9 TR 10@+-1@+
 2 9 BR oo
 6 9 MR oo
10 9 TR oo
%
psbasemap -J -R0/1.2/0/1.2 -B0.1/0.1NE -O >> $ps

