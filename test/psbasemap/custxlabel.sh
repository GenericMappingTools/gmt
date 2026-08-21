#!/usr/bin/env bash
# Test Cartesian, relative and absolute time axis custom annotations
# As of 6/16/2016 the relative time example fails.  I faked the orig PS
# by running the absolute time part twice (the commented out commands).
# Once the bug is fixed we will remove the commented-out versions
# 7/15/2016: Test now passes - I left the comments in there.

ps=custxlabel.ps
# Cartesian
cat << EOF > m.txt
0		afg	zero
1.61803		afg	@~j@~
2.71828		afg	e
3.1415926 	afg	@~p@~
5	 	f
6.283186	afg	@~2p@~
EOF
gmt psbasemap -R-1/7/-1/7 -JX6i/2.75i -P -Bxcm.txt -Byafg -BWSne -K -Xc > $ps
# Time axis relative time
cat << EOF > m.txt
10 a p@-1@-
40 afg p@-2@-
50 f
81 a p@-3@-
EOF
gmt psbasemap -R2013-06-01T/2013-08-31T/-10/10 -JX6it/2.75i --TIME_EPOCH=2013-05-31T --TIME_UNIT=d -Byafg -Bxcm.txt -BWSne -O -K -Y3.25i >> $ps
#cat << EOF > m.txt
#2013-06-10T a p@-1@-
#2013-07-10T afg p@-2@-
#2013-07-20T f
#2013-08-20T a p@-3@-
#EOF
#gmt psbasemap -R2013-06-01T/2013-08-31T/-10/10 -JX6iT/2.75i -Byafg -Bxcm.txt -BWSne -O -K -Y3.25i  >> $ps
# Time axis absolute time
cat << EOF > m.txt
2013-06-10T a p@-1@-
2013-07-10T afg p@-2@-
2013-07-20T f
2013-08-20T a p@-3@-
EOF
gmt psbasemap -R2013-06-01T/2013-08-31T/-10/10 -JX6iT/2.75i -Byafg -Bxcm.txt -BWSne -O -Y3.25i  >> $ps
