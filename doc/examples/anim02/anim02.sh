#!/usr/bin/env bash
#               GMT ANIMATION 02
#
# Purpose:      Make simple animated GIF of an illuminated DEM grid
# GMT modules   math, makecpt, grdimage, plot, movie
# Unix progs:   cat
# Note:         Run with any argument to build movie; otherwise 1st frame is plotted only.

if [ $# -eq 0 ]; then	# Just make master PostScript frame 0
	opt="-Mps -Fnone"
	ps=anim02.ps
else	# Make animated GIF
	opt="-A+l"
fi	
# 1. Create files needed in the loop
cat << EOF > pre.sh
gmt begin
	gmt math -T0/360/10 T 180 ADD = angles.txt
	gmt makecpt -Crainbow -T500/4500 -H > main.cpt
gmt end
EOF
# 2. Set up the main frame script
cat << EOF > main.sh
gmt begin
	width=\`gmt math -Q \${MOVIE_WIDTH} 0.5i SUB =\`
	gmt grdimage @tut_relief.nc -I+a\${MOVIE_COL0}+nt2 -JM\${width} -Cmain.cpt \
		-BWSne -B1 -X0.35i -Y0.3i --FONT_ANNOT_PRIMARY=9p
	gmt plot -Sc0.8i -Gwhite -Wthin <<< "256.25 35.6" 
	gmt plot -Sv0.1i+e -Gred -Wthick <<< "256.25 35.6 \${MOVIE_COL1} 0.37i" 
gmt end
EOF
# 3. Run the movie
gmt movie main.sh -C3.5ix4.167ix72 -Nanim02 -Tangles.txt -Sbpre.sh -D6 -Z $opt
rm -rf main.sh pre.sh
