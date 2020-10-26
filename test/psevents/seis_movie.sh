#!/bin/bash
# Recipe for making equidistant in time point file
# Pick point spacing for a ${s}p line width
R=-R1984-09-10T03:15/1984-09-10T03:45/-15/15
dpi=200
s=0.25
h=4
L=S
x=1
gmt set FORMAT_FLOAT_OUT %.16g
d=$(gmt math -Q $dpi INV $x MUL =)
gmt mapproject onkl_waveform.dat $R -JX20cT/10c -G+uC --PROJ_LENGTH_UNIT=inch | gmt sample1d -N2 -T${d}c -Fl -AR | gmt mapproject $R -JX20cT/10c -I -o0-2,0 --FORMAT_CLOCK_OUT=hh:mm:ss.xx --PROJ_LENGTH_UNIT=inch > pts.txt
cat << EOF > main.sh
gmt begin map_E${dpi}_${L}${s}p_H${h}_X${x} png E${dpi},H${h}
	gmt set FORMAT_DATE_MAP "o dd yyyy" FORMAT_CLOCK_MAP hh:mm FONT_ANNOT_PRIMARY +9p
	gmt basemap $R -JX20cT/10c -Bpxaf -Bsxa1D -Bpyafg100 -BWSEn --TIME_INTERVAL_FRACTION=0.01 --FORMAT_CLOCK_MAP=hh:mm:ss
	#gmt plot onkl_waveform.dat  -Sc1p -Gblack
	echo E${dpi},${L}${s}p,H${h},X${x} | gmt text -F+cTL+f14p -Dj0.5c
	if [ "X${L}" = "XW" ]; then
		gmt plot onkl_waveform.dat $R -W${s}p,red
	else
		gmt plot pts.txt -Sc${s}p -Gred
	fi
gmt end show
EOF
chmod +x main.sh
main.sh
