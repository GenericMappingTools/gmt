#!/usr/bin/env bash
#               GMT ANIMATION 08
#
# Purpose:      Show one year (2018) of Pacific seismicity events
# GMT modules:  convert, math, makecpt, movie, coast, plot, events
# Unix progs:   cat
# Note:         Run with any argument to build movie; otherwise one frame is plotted only.

if [ $# -eq 0 ]; then   # Just make master PostScript frame 100
	opt="-M100,ps -Fnone"
	ps=anim08.ps
else    # Make movie in MP4 format and a thumbnail animated GIF using every 5th frame
	opt="-Fmp4 -A+l+s5"
fi
# Test reading directly from URL
# Note: The URL tends to change every few years...
# 1. Extract 2018 data from URL and prepare inputs and frame times
cat << EOF > pre.sh
SITE="https://earthquake.usgs.gov/fdsnws/event/1/query.csv"
TIME="starttime=2018-01-01%2000:00:00&endtime=2018-12-31%2000:00:00"
MAG="minmagnitude=5"
ORDER="orderby=time-asc"
URL="\${SITE}?\${TIME}&\${MAG}&\${ORDER}"
gmt begin
	gmt convert \$URL -i2,1,3,4+s50,0 -hi1 > q.txt
	gmt makecpt -Cred,green,blue -T0,70,300,10000 -H > movie_dem.cpt
	gmt math -T2018-01-01T/2018-12-31T/2 --TIME_UNIT=d TNORM 40 MUL 200 ADD = times.txt
gmt end
EOF
# 2. Set up main script
cat << EOF > main.sh
gmt begin
	gmt coast -Rg -JG\${MOVIE_COL1}/5/6i -G128 -S32 -X0 -Y0 -A500
	gmt plot @ridge.txt -W0.5p,darkyellow
	gmt events q.txt -SE- -Cmovie_dem.cpt --TIME_UNIT=d -T\${MOVIE_COL0} -Es+r2+d6 -Ms5+c0.5 -Mi1+c-0.6 -Mt+c0
gmt end
EOF
# 3. Run the movie
gmt movie main.sh -Sbpre.sh -C6ix6ix100 -Ttimes.txt -Nanim08 -Gblack -H2 -Z -Lc0+f20p,Helvetica,white --FORMAT_CLOCK_MAP=- $opt
rm -rf main.sh pre.sh
