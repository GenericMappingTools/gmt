#!/usr/bin/env bash
#
ps=seis_11.ps
# Basic demo of events using meca - here we only show the master frame but
# you can debug the movie making by changing -Fnone to -Fmp4
gmt makecpt -T0/30 -Cbatlow > t.cpt
cat << EOF > data.txt
3 3 20  4.319 37.28 145.8  6 2020-02-01T 0 100% double-couple [-Sa]
EOF
cat << EOF > labels.txt
0	0	2020-01-31T		RISE
0	0	2020-02-01T		PLATEAU
0	0	2020-02-02T		DECAY
0	0	2020-02-03T		NORMAL
0	0	2020-02-04T		FADE
0	0	2020-02-05T		CODA
EOF
gmt math -T2020-01-31T/2020-02-06T/2 --TIME_UNIT=h -o0 T = time.txt
cat << EOF > main.sh
gmt begin
	gmt basemap -JM5i -R0/6/0/6 -B -X0.5i -Y0.5i
	gmt events data.txt -Z"psmeca -Sa4c+f0" -Ct.cpt -W1p -T\${MOVIE_COL0} -Es+r1+p1+d1+f1 -Mi1+c-0.5 -Ms2+c0.5 -Mt+c0 -L3 --TIME_UNIT=d
	gmt events labels.txt -L1 -Et -T\${MOVIE_COL0} -F+f12p,Helvetica-Bold,red+jBL -Dj0.1c --TIME_UNIT=d
gmt end
EOF
#gmt movie main.sh -C6ix6ix100 -Nseis_11 -Fmp4 -Mm,ps -Lc0+o1.5c -Lf+jTR+o1.5c -Ttime.txt -Z --FORMAT_CLOCK_MAP=hh
gmt movie main.sh -C6ix6ix100 -Nseis_11 -Fnone -Mm,ps -Lc0+o1.5c -Lf+jTR+o1.5c -Ttime.txt -Z --FORMAT_CLOCK_MAP=hh
rm -f t.cpt time.txt data.txt labels.txt main.sh
