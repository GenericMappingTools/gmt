#!/usr/bin/env bash
#
# This script takes the downloaded zip content of the GMT-formatted
# cmocean colour maps by Kirsten Thyng from
# https://github.com/kthyng/cmocean-gmt/archive/refs/heads/master.zip
# (Last version 2.0) and converts the included CPT files to the
# proper GMT master CPT files with the correct attribution and hinge info.
#
# Run from the cmoceans-V directory (V is version) after the tarball has
# downloaded and been expanded. But first you need to manually update the
# the /tmp/cpt.info entries below with potentially new colour maps.
# The script will create a gmt_cpts subdirectory with all the CPTs.
# You also need to edit gmt_cpt_masters.h after adding the CPTs to share/cpt.
#
# Last setup and run on 01-Feb-2022 for GMT 6.4 (master).
# This gives 21 CPTs. Note that "gray" is excluded, since GMT already has it.
#

if [ $# -eq 0 ]; then
	cat <<- EOF  >&2
	Usage: build-cmocean-cpt.sh <cmocean-directory>

	Will create the GMT CPT versions of Thyng's ocean colour maps.
	Give the path to the expanded tarball top directory, such as
	~/Download/cmocean-gmt-master
	Before running you must update this script with:
	  1. Any new CPT entries since the last release to /tmp/cpt.info
	  2. Flag those with a soft hinge as S and a hard hinge as H
	  3. Manually set the current version number
	Afterwards you must:
	  1. Update src/gmt_cpt_masters.h with any new entries (copy lines from /tmp/cpt_strings.txt)
	  2. Adding the CPTs to share/cpt (overwriting the previous versions)
	  3. Probably mess with doc/scripts/GMT_App_M*.sh for new layouts
	EOF
	exit 1
fi

DIR=$1
VERSION=2.0
cat << EOF > /tmp/cpt.info
algae|Perceptually uniform colormap, by Kirsten Thyng [C=RGB]
amp|Perceptually uniform colormap, by Kirsten Thyng [C=RGB]
balance|Perceptually uniform divergent colormap, by Kirsten Thyng [S,C=RGB]
curl|Perceptually uniform divergent colormap, by Kirsten Thyng [S,C=RGB]
deep|Perceptually uniform colormap, by Kirsten Thyng [C=RGB]
delta|Perceptually uniform divergent colormap, by Kirsten Thyng [S,C=RGB]
dense|Perceptually uniform colormap, by Kirsten Thyng [C=RGB]
diff|Perceptually uniform divergent colormap, by Kirsten Thyng [S,C=RGB]
gray|Perceptually uniform grayscale map, by Kirsten Thyng [C=RGB]
haline|Perceptually uniform colormap, by Kirsten Thyng [C=RGB]
ice|Perceptually uniform colormap, by Kirsten Thyng [C=RGB]
matter|Perceptually uniform colormap, by Kirsten Thyng [C=RGB]
oxy|Perceptually uniform divergent colormap, by Kirsten Thyng [C=RGB]
phase|Perceptually uniform cyclic colormap, by Kirsten Thyng [O,C=RGB]
rain|Perceptually uniform colormap, by Kirsten Thyng [C=RGB]
solar|Perceptually uniform colormap, by Kirsten Thyng [C=RGB]
speed|Perceptually uniform colormap, by Kirsten Thyng [C=RGB]
tarn|Perceptually uniform colormap, by Kirsten Thyng [S,C=RGB]
tempo|Perceptually uniform colormap, by Kirsten Thyng [C=RGB]
thermal|Perceptually uniform colormap, by Kirsten Thyng [C=RGB]
topo|Perceptually uniform colormap, by Kirsten Thyng [H,C=RGB]
turbid|Perceptually uniform colormap, by Kirsten Thyng [C=RGB]
EOF

cd $DIR
# Make formatted list of lines suitable for copying into gmt_cpt_masters.h
awk -F'|' '{printf "\"cmocean/%-10s : %s\",\n", $1, $2}' /tmp/cpt.info > /tmp/cpt_strings.txt

rm -rf gmt_cpts
mkdir -p gmt_cpts/cmocean
while read line; do
	cpt=$(echo $line | cut -d'|' -f1)
	in=cpt/$cpt.cpt
	out=gmt_cpts/cmocean/$cpt.cpt
	hinge=0
	echo $line | awk -F'|' '{printf ("#\n# cmocean/%s : %s\n", $1, $2)}' > $out
	cat <<- EOF >> $out
	#
	# https://github.com/kthyng/cmocean-gmt, version $VERSION
	#
	# License: MIT License
	# Copyright (c) 2015, Kirsten M. Thyng
	# Thyng, K. M., Greene, C. A., Hetland, R. D., Zimmerle, H. M., & DiMarco, S. F. (2016). True colors of oceanography. Oceanography, 29(3), 10.
	# http://tos.org/oceanography/assets/docs/29-3_thyng.pdf
	#
	# Note: Original file converted to GMT version >= 5 CPT format.
	#
	#----------------------------------------------------------
	# COLOR_MODEL = RGB
	EOF
	case $line in
	*\[H*) hinge=1; echo "# HARD_HINGE" >> $out ;;
	*\[S*) hinge=1; echo "# SOFT_HINGE" >> $out ;;
	*\[O*) echo "# CYCLIC" >> $out ;;
	esac
	echo "#----------------------------------------------------------" >> $out
	if test $hinge == 0; then
		egrep -v '^#' $in | awk '{printf "%.6f\t%s/%s/%s\t%.6f\t%s/%s/%s\n", ($1-1)/256, $2, $3, $4, ($5-1)/256, $6, $7, $8}' >> $out
	else
		egrep -v '^#' $in | awk '{printf "%.6f\t%s/%s/%s\t%.6f\t%s/%s/%s\n", ($1-129)/128, $2, $3, $4, ($5-129)/128, $6, $7, $8}' >> $out
	fi
done < /tmp/cpt.info

echo "Folder with new cpts is $DIR/gmt_cpts"
