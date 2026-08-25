#!/usr/bin/env bash
#
# Create original PS plot and PNG image that include these GMT features:
# a) patterns
# b) image
# c) transparency
# d) lines and annotations, including degree symbol
#
# Then run the new candidate gs with suitable options to create a PNG we can compare
# using gmt compare to the original PNG that we have verified manually.
# Run from the top gmt-dev directory

if [ ! -d cmake ]; then
	echo "Must be run from top-level gmt directory"
	exit 1
fi
if [ ! -d build ]; then
	mkdir -p build
fi
# Do the work in the build directory
echo "Working in build directory"
cd build
if [ ! -f ../admin/PS_orig_for_gs.ps ]; then
	# Make the original GMT PS plot if it is not present
	gmt begin PS_orig_for_gs ps
		gmt set PS_MEDIA letter
		gmt grdimage -R0/20/0/20 -JM16c @earth_relief_05m -B -B+t"GMT TEST PLOT" -I+d
		echo 10 10 | gmt plot -Ss6c -Gp20+r100+fblue -W2p
		echo 4 4 | gmt plot -Ss6c -Gwhite@50 -W2p
		echo 16 16 | gmt plot -Ss6c -Gwhite -W2p
	gmt end
	# Place the original PS in admin
	mv PS_orig_for_gs.ps ../admin
	echo "Add admin/PS_orig_for_gs.ps to GitHub as the basis for comparisons"
fi
if [ ! -f ../admin/PS_orig_for_gs.png ]; then
	# Create the original PNG with transparency via PDF if it is not present
	gmt psconvert ../admin/PS_orig_for_gs.ps -TG
	rm -f ../admin/PS_orig_for_gs_intermediate.pdf
	echo "Add admin/PS_orig_for_gs.png to GitHub as the basis for comparisons"
fi

# Test if current gs version can reproduce our original
echo "Convert PS_orig_for_gs.ps to PS_orig_for_gs.pdf in order to achieve transparency"
gs -q -dNOPAUSE -dBATCH -dNOSAFER -dPDFSETTINGS=/prepress -dDownsampleColorImages=false -dDownsampleGrayImages=false \
  -dDownsampleMonoImages=false -dUseFlateCompression=true -dEmbedAllFonts=true -dSubsetFonts=true -dMonoImageFilter=/FlateEncode \
  -dAutoFilterGrayImages=false -dGrayImageFilter=/FlateEncode -dAutoFilterColorImages=false -dColorImageFilter=/FlateEncode \
  -dSCANCONVERTERTYPE=2 -dALLOWPSTRANSPARENCY -dMaxBitmap=2147483647 -dUseFastColor=true -dGraphicsAlphaBits=1 -dTextAlphaBits=1 \
  -sDEVICE=pdfwrite  -g2550x3300 -r300 -sOutputFile='PS_orig_for_gs_intermediate.pdf' '../admin/PS_orig_for_gs.ps'
echo "Convert PS_orig_for_gs.pdf to PS_orig_for_gs.png"
gs -q -dNOPAUSE -dBATCH -dNOSAFER -dPDFSETTINGS=/prepress -dDownsampleColorImages=false -dDownsampleGrayImages=false \
  -dDownsampleMonoImages=false -dUseFlateCompression=true -dEmbedAllFonts=true -dSubsetFonts=true -dMonoImageFilter=/FlateEncode \
  -dAutoFilterGrayImages=false -dGrayImageFilter=/FlateEncode -dAutoFilterColorImages=false -dColorImageFilter=/FlateEncode \
  -dSCANCONVERTERTYPE=2 -dALLOWPSTRANSPARENCY -dMaxBitmap=2147483647 -dUseFastColor=true -dGraphicsAlphaBits=2 -dTextAlphaBits=4 \
  -sDEVICE=pngalpha  -g2550x3300 -r300 -sOutputFile='PS_orig_for_gs.png' 'PS_orig_for_gs_intermediate.pdf'

echo "Compare PS_orig_for_gs.png to ../admin/PS_orig_for_gs.png"
gm compare -density 200 -maximum-error 0.003 -highlight-color magenta -highlight-style assign -metric rmse -file diff.png ../admin/PS_orig_for_gs.png PS_orig_for_gs.png
cd ..
