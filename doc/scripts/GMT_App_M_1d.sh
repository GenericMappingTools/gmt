#!/usr/bin/env bash
#
#	Makes the insets for Appendix M(cpt)
#	[skip srtm which is just a special version of dem2]
#
# We have five sets of CPT figures to make:
# 1a) Our regular, traditional GMT CPTs [46]
# 1b) The regular Scientific Color Maps* [30]
# 1c) Cyclic CPTs [7]
# 1d) Colormaps from cmocean [22]
#
# *from Fabio [www.fabiocrameri.ch/visualisation]

plot () {
	gmt makecpt -H -C$1 -T-1/1 > tt.1.cpt
	gmt makecpt -H -C$1 -T-1/1/0.25 > tt.2.cpt
	gmt colorbar -Dx${2}i/${y}i+w2.70i/0.125i+h+jTC+e -Ctt.1.cpt -B0
	gmt colorbar -Dx${2}i/${y2}i+w2.70i/0.125i+h+jTC+e -Ctt.2.cpt -Bf0.25
	echo $2 $y $1 | gmt text -D0/0.05i -F+f9p,Helvetica-Bold+jBC
	if [ $(grep -c RANGE ${GMT_SHAREDIR}/cpt/$1.cpt) -eq 1 ]; then # Plot default range for left CPT
		grep RANGE ${GMT_SHAREDIR}/cpt/$1.cpt | awk '{printf "2.9 %g %s\n", "'$y'", $4}' | gmt text -F+f6p,Helvetica+jRB -D0/0.025i -N
	fi
	if [ $(grep -c HARD_HINGE ${GMT_SHAREDIR}/cpt/$1.cpt) -eq 1 ]; then # Plot hard hinge symbol for left CPT
		echo $2 $y | gmt plot -St0.2c -Gblack -Wfaint -D0/-0.29i
	elif [ $(grep -c SOFT_HINGE ${GMT_SHAREDIR}/cpt/$1.cpt) -eq 1 ]; then # Plot soft hinge symbol for left CPT
		echo $2 $y | gmt plot -St0.2c -Gwhite -Wfaint -D0/-0.29i
	fi
}

GMT_SHAREDIR=$(gmt --show-sharedir)

# Here we list all the cmocean cpts:
sed -e 's/"//g' "${GMT_SOURCE_DIR}"/src/gmt_cpt_masters.h | egrep cmocean | awk '{print $1}' | sort > tt.lis

n=$(cat tt.lis | wc -l)
# dy is line spacing and y0 is total box height
dy=0.6
y0=$(gmt math -Q $n $dy MUL 0.5 MUL 0.25 ADD =)

gmt begin GMT_App_M_1d
gmt set MAP_FRAME_PEN thinner FONT_ANNOT_PRIMARY 8p MAP_TICK_LENGTH_PRIMARY 0.1i MAP_ANNOT_OFFSET_PRIMARY 0.04i
gmt basemap -R0/6.1/0/$y0 -Jx1i -B0

i=1
y=0.375
y2=0.25
while [ $i -le $n ]
do
	if [ $i -eq $n ]; then
		plot $(sed -n 1p tt.lis) 3.05
	else
		plot $(sed -n $(expr $n - $i    )p tt.lis) 1.55
		plot $(sed -n $(expr $n - $i + 1)p tt.lis) 4.50
	fi
	i=$(expr $i + 2)
	y=$(gmt math -Q $y $dy ADD =)
	y2=$(gmt math -Q $y2 $dy ADD =)
done
rm -f tt.*
gmt end show
