#!/usr/bin/env bash
#
ps=seis_12.ps
# Basic demo of events using coupe - here we only show the master frame but
# you can debug the movie making by changing -Fnone to -Fmp4
gmt makecpt -T30/70 -Cturbo > t.cpt
cat << EOF > data.txt
# lon  lat  dep str dip rake str dip rake m ex [nx ny label]
129.5 10.5  66  0   90   0  90   90 180   1 24  2020-02-01T00  0  0 66 km
128.5 10.5  46  0   45  90 180   45  90   1 24  2020-02-01T12  0  0 46 km
EOF
cat << EOF > labels.txt
0	100	2020-01-31T		RISE
0	100	2020-02-01T		PLATEAU
0	100	2020-02-02T		DECAY
0	100	2020-02-03T		NORMAL
0	100	2020-02-04T		FADE
0	100	2020-02-05T		CODA
EOF
gmt math -T2020-01-31T/2020-02-06T/2 --TIME_UNIT=h -o0 T = time.txt
cat << EOF > main.sh
gmt begin
	gmt basemap -R0/250/0/100 -Jx0.02i/-0.02i -B -Y0.5i -X0.5i
	gmt events labels.txt -L1 -Et -T\${MOVIE_COL0} -F+f12p,Helvetica-Bold,red+jBL -Dj0.1c --TIME_UNIT=d
	gmt events -Z"coupe -Q -L -Sc3c -Ab128/11/120/250/90/400/0/100+f -Fa0.1i/cc" -Ct.cpt -N data.txt -T\${MOVIE_COL0} -Es+r1+p1+d1+f1 -Mi1+c-0.5 -Ms2+c0.5 -Mt+c0 -L3 --TIME_UNIT=d
gmt end
EOF
#gmt movie main.sh -C6ix3ix100 -Nseis_12 -Fmp4 -Mm,ps -Lc0+jTR+o1.5c -Lf+o1.5c -Ttime.txt -Z --FORMAT_CLOCK_MAP=hh
gmt movie main.sh -C6ix3ix100 -Nseis_12 -Fnone -Mm,ps -Lc0+jTR+o1.5c -Lf+o1.5c -Ttime.txt -Z --FORMAT_CLOCK_MAP=hh
rm -f t.cpt time.txt data.txt labels.txt main.sh
