#!/usr/bin/env bash
#
# This script takes the downloaded zip content from
# Crameri, Fabio. (2020, January 6). Scientific colour maps
# (Version 6.0.4). Zenodo. http://doi.org/10.5281/zenodo.4153113
# and converts the *.cpt files into proper GMT master
# CPT files with correct attribution and hinge info
# Run from the ScientificColourMapsX directory (X is version) after the
# zip has downloaded and been expanded.  But first you need to manually
# update the /tmp/cpt.info entries below with one line per CPT subdir in the
# downloaded directory.  It will create a gmt subdirectory with all the CPTs.
# You also need to edit gmt_cpt_masters.h after adding the CPTs to share/cpt
#
# Last setup and run for ScientificColourMaps6 on 11/08/2020 for GMT 6.2 (master)
# Gave 44 CPTS: The original 24 plus 4 cyclical versions and 16 categorical versions
#

if [ $# -eq 0 ]; then
	cat <<- EOF  >&2
	Usage: build-scientific-colors-cpt.sh <SCM-directory>
	
	Will create the GMT CPT versions of Crameri's scientific colour maps.
	Give the full path to the expanded zip file top directory, such as
	~/Download/ScientificColourMaps6.
	Before running you must update this script with:
	  1. Any new CPT entries since his last release to /tmp/cpt.info
	  2. Flag those with a soft hinge as S and a hard hinge as H
	  3. Manually set the current version number/doi (see the zip PDF docs)
	Afterwards you must:
	  1. Update gmt_cpt_masters.h with any new entries (see /tmp/cpt_strings.txt)
	  2. Adding the CPTs to share (overwriting the previous versions)
	  3. Probably mess with doc/scripts/GMT_App_M*.sh for new layout
	EOF
	exit 1
fi

DIR=$1
VERSION=6.0.4
cat << EOF > /tmp/cpt.info
acton|Perceptually uniform sequential colormap, by Fabio Crameri [C=RGB]
actonS|Perceptually uniform sequential categorical colormap, by Fabio Crameri [C=RGB]
bamako|Perceptually uniform, low-lightness gradient colormap by Fabio Crameri [C=RGB]
bamakoS|Perceptually uniform, low-lightness gradient categorical colormap by Fabio Crameri [C=RGB]
batlow|Perceptually uniform 'rainbow' colormap by Fabio Crameri [C=RGB]
batlowS|Perceptually uniform 'rainbow' categorical colormap by Fabio Crameri [C=RGB]
berlin|Perceptually uniform bimodal colormap, dark, by Fabio Crameri [S,C=RGB]
bilbao|Perceptually uniform colormap by Fabio Crameri [C=RGB]
bilbaoS|Perceptually uniform categorical colormap by Fabio Crameri [C=RGB]
broc|Perceptually uniform bimodal colormap, light, by Fabio Crameri [S,C=RGB]
brocO|Perceptually uniform bimodal cyclic colormap, light, by Fabio Crameri [C=RGB]
buda|Perceptually uniform, low-lightness gradient colormap, by Fabio Crameri [C=RGB]
budaS|Perceptually uniform, low-lightness gradient categorical colormap, by Fabio Crameri [C=RGB]
cork|Perceptually uniform bimodal colormap, light, by Fabio Crameri [S,C=RGB]
corkO|Perceptually uniform bimodal cyclic colormap, light, by Fabio Crameri [C=RGB]
davos|Perceptually uniform colormap by Fabio Crameri [C=RGB]
davosS|Perceptually uniform categorical colormap by Fabio Crameri [C=RGB]
devon|Perceptually uniform sequential colormap, by Fabio Crameri [C=RGB]
devonS|Perceptually uniform sequential categorical colormap, by Fabio Crameri [C=RGB]
grayC|Perceptually uniform 'gray' colormap by Fabio Crameri [C=RGB]
grayCS|Perceptually uniform 'gray' categorical colormap by Fabio Crameri [C=RGB]
hawaii|Perceptually uniform lush colormap by Fabio Crameri [C=RGB]
hawaiiS|Perceptually uniform lush categorical colormap by Fabio Crameri [C=RGB]
imola|Perceptually uniform, low-lightness gradient colormap, by Fabio Crameri [C=RGB]
imolaS|Perceptually uniform, low-lightness gradient categorical colormap, by Fabio Crameri [C=RGB]
lajolla|Perceptually uniform colormap, without black or white, by Fabio Crameri [C=RGB]
lajollaS|Perceptually uniform categorical colormap, without black or white, by Fabio Crameri [C=RGB]
lapaz|Perceptually uniform 'rainbow' colormap by Fabio Crameri [C=RGB]
lapazS|Perceptually uniform 'rainbow' categorical colormap by Fabio Crameri [C=RGB]
lisbon|Perceptually uniform bimodal colormap, dark, by Fabio Crameri [S,C=RGB]
nuuk|Perceptually uniform, low-lightness gradient colormap, by Fabio Crameri [C=RGB]
nuukS|Perceptually uniform, low-lightness gradient categorical colormap, by Fabio Crameri [C=RGB]
oleron|Perceptually uniform topography colormap, by Fabio Crameri [H,C=RGB]
oslo|Perceptually uniform, B&W limits, by Fabio Crameri [C=RGB]
osloS|Perceptually uniform, B&W limits, categorical colormap, by Fabio Crameri [C=RGB]
roma|Perceptually uniform 'seis' colormap by Fabio Crameri [C=RGB]
romaO|Perceptually uniform cyclic colormap by Fabio Crameri [C=RGB]
tofino|Perceptually uniform bimodal colormap, dark, by Fabio Crameri [S,C=RGB]
tokyo|Perceptually uniform colormap without black or white, by Fabio Crameri [C=RGB]
tokyoS|Perceptually uniform categorical colormap without black or white, by Fabio Crameri [C=RGB]
turku|Perceptually uniform colormap by Fabio Crameri [C=RGB]
turkuS|Perceptually uniform categorical colormap by Fabio Crameri [C=RGB]
vik|Perceptually uniform bimodal colormap, light, by Fabio Crameri [S,C=RGB]
vikO|Perceptually uniform bimodal cyclic colormap, light, by Fabio Crameri [C=RGB]
EOF
here=`pwd`
cd $DIR
awk -F'|' '{printf "\"%-10s : %s\",\n", $1, $2}' /tmp/cpt.info > /tmp/cpt_strings.txt
rm -rf gmt_cpts
mkdir gmt_cpts
cat <<- EOF > /tmp/front
#
#----------------------------------------------------------
# COLOR_MODEL = RGB
EOF
while read line; do
	cpt=`echo $line | awk -F'|' '{print $1}'`
	cat <<- EOF > gmt_cpts/$cpt.cpt
	#
	EOF
	echo $line | awk -F'|' '{printf "# %s\n", $2}' >> gmt_cpts/$cpt.cpt
	last_char=$(echo $cpt | awk '{print substr($1,length($1),1)}')
	if [ "X${last_char}" = "XS" ]; then
		tmp=$(echo $cpt | awk '{print substr($1,1, length($1)-1)}')
		cptdir=${tmp}/CategoricalPalettes
	else
		cptdir=${cpt}
	fi
	cat <<- EOF >> gmt_cpts/$cpt.cpt
	#
	#	www.fabiocrameri.ch/visualisation
	#
	# License: Creative Commons Attribution 4.0 International License
	# Copyright (c) 2020, Fabio Crameri All rights reserved.
	# Crameri, F., (2020). Scientific colour maps. Zenodo. https://zenodo.org/record/4153113
	# This is Scientific Colour Maps version $VERSION
	# Note: Original file converted to GMT version >= 5 CPT format.
	EOF
	#if [ "$cpt" = "broc" ] || [ "$cpt" = "cork" ] || [ "$cpt" = "vik" ] || [ "$cpt" = "lisbon" ] || [ "$cpt" = "tofino" ] || [ "$cpt" = "berlin" ] || [ "$cpt" = "oleron" ] ; then
	if [ $(echo $line | grep -c "\[H") -eq 1 ]; then
		hinge="HARD_HINGE"
		echo "Remember to manually fix the hard hinge for $cpt"
	elif [ $(echo $line | grep -c "\[S") -eq 1 ]; then
		hinge="SOFT_HINGE"
	else
		hinge=""
	fi
	if [ "X${last_char}" = "XS" ]; then
		cat /tmp/front >> gmt_cpts/$cpt.cpt
		echo "#----------------------------------------------------------" >> gmt_cpts/$cpt.cpt
		egrep -v '^#|^F|^B|^N' $cptdir/$cpt.cpt | awk '{if (NR == 1) { printf "%d\t%s/%s/%s\n%d\t%s/%s/%s\n", 0, $2, $3, $4, 1, $6, $7, $8} else {printf "%d\t%s/%s/%s\n", NR+1, $6, $7, $8}}' > /tmp/tmp.cpt 
	elif [ "X$hinge" = "X" ]; then
		cat /tmp/front >> gmt_cpts/$cpt.cpt
		if [ "X${last_char}" = "XO" ]; then
			echo "# CYCLIC" >> gmt_cpts/$cpt.cpt
		fi
		echo "#----------------------------------------------------------" >> gmt_cpts/$cpt.cpt
		egrep -v '^#|^F|^B|^N' $cptdir/$cpt.cpt | awk '{printf "%.6f\t%s/%s/%s\t%.6f\t%s/%s/%s\n", $1, $2, $3, $4, $5, $6, $7, $8}' > /tmp/tmp.cpt 
	else
		echo "# Note: Range changed from 0-1 to -1/+1 to place hinge at zero." >> gmt_cpts/$cpt.cpt
		cat /tmp/front >> gmt_cpts/$cpt.cpt
		echo "# $hinge" >> gmt_cpts/$cpt.cpt
		echo "#----------------------------------------------------------" >> gmt_cpts/$cpt.cpt
		# Convert to -1/1 range
		egrep -v '^#|^F|^B|^N' $cptdir/$cpt.cpt | awk '{printf "%.6f\t%s/%s/%s\t%.6f\t%s/%s/%s\n", 2*($1-0.5), $2, $3, $4, 2*($5-0.5), $6, $7, $8}' > /tmp/tmp.cpt 
	fi
	cat /tmp/tmp.cpt >> gmt_cpts/$cpt.cpt
	if [ "X${last_char}" = "XS" ] || [ "X${last_char}" = "XO" ]; then	# Categorical or cyclical CPTS have no F or B, only NaN
		egrep '^N' $cptdir/$cpt.cpt | awk '{printf "%s\t%s/%s/%s\n", $1, $2, $3, $4}' >> gmt_cpts/$cpt.cpt
	else
		egrep '^F|^B|^N' $cptdir/$cpt.cpt | awk '{printf "%s\t%s/%s/%s\n", $1, $2, $3, $4}' >> gmt_cpts/$cpt.cpt
	fi
done < /tmp/cpt.info
# Fix the zero hinge for oleron
grep '^#' gmt_cpts/oleron.cpt > /tmp/oleron.cpt
egrep -v '^#|B|N|F' gmt_cpts/oleron.cpt | awk '{if (NR == 127) {printf "%s\t%s\t0.0\t\t\t%s\n", $1, $2, $4} else if (NR == 129) {printf "0.0\t\t\t%s\t%s\t%s\n", $2, $3, $4} else if (NR != 128) { print $0}}' >> /tmp/oleron.cpt
egrep '^B|^N|^F' gmt_cpts/oleron.cpt >> /tmp/oleron.cpt
mv -f /tmp/oleron.cpt gmt_cpts

rm -f tmp
cd $here
echo "Folder with new cpts is $DIR/gmt_cpts"
