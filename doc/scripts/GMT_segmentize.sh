#!/usr/bin/env bash
# Display 6 effect of segmentizing option -F in plot with geographic data

function plotpts
{	# Plots the two data tables and places given text
	gmt plot -Sc5p -Ggreen -Wfaint t1.txt
	gmt plot -Sc5p -Gblue  -Wfaint t2.txt
	gmt text -F+cTL+jTL+f7p,Helvetica-Bold+t"$*" -Dj3p
}
cat << EOF > t1.txt
10	10
48	15
28	20
>
40	40
30	5
5	15
EOF
cat << EOF > t2.txt
7	20
29	11
8	4
EOF

gmt begin GMT_segmentize
	gmt set GMT_THEME cookbook
	gmt set FONT_ANNOT_PRIMARY 9p
	gmt subplot begin 2x3 -Fs5c/4.5c -R0/50/0/45  -Sct -Srl -Jx0.1cd -M1p
		# Show the data and its natural connectivity
		gmt plot -W0.25p,- t[12].txt -c -l"Data connections"+jTL+o2p/12p+gwhite+f6p
		printf "5 35\n20 35\n" | gmt plot -W1p -l"New connections"
		gmt plot -Sc5p -Ggreen -Wfaint t1.txt -l"Table 1"+jTL+o2p/12p+gwhite
		gmt plot -Sc5p -Gblue  -Wfaint t2.txt -l"Table 2"
		gmt text -F+cTL+jTL+f7p,Helvetica-Bold+t"TWO DATA TABLES" -Dj3p
		# Lines from dataset origin
		gmt plot -W0.25p,- t[12].txt -c
		gmt plot -W1p t[12].txt -Fra
		plotpts DATASET ORIGIN
		# Lines from table origin
		gmt plot -W0.25p,- t[12].txt -c
		gmt plot -W1p t[12].txt -Frf
		plotpts TABLE ORIGIN
		# Lines from segment origin
		gmt plot -W0.25p,- t[12].txt -c
		gmt plot -W1p t[12].txt -Frs
		plotpts SEGMENT ORIGIN
		# Lines from fixed origin
		gmt plot -W0.25p,- t[12].txt -c
		gmt plot -W1p t[12].txt -Fr10/35
		plotpts FIXED ORIGIN
		echo 10 35 | gmt plot -Sa9p -Gred -Wfaint
		# Lines for network
		gmt plot -W0.25p,- t[12].txt -c
		gmt plot -W1p t[12].txt -Fna
		plotpts NETWORK
	gmt subplot end
gmt end show
