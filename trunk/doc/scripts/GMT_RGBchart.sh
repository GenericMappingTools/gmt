#!/bin/bash
#	$Id$
#
# Plots a page of all 555 unique named colors
# Usage: GMT_RGBchart.sh <size>
# where <size> is the page size. Use either: ledger, a4, or letter
# This produces the file GMT_RGBchart_<size>.ps

. ./functions.sh
. gmt_shell_functions.sh

gmt_init_tmpdir

SIZE=$1
COL=16
ROW=35

if [ $SIZE = letter ] ; then
	WIDTH=10.5
	HEIGHT=8.0
	ORIENT=landscape
elif [ $SIZE = a4 ] ; then
	WIDTH=11.2
	HEIGHT=7.8
else
	SIZE=tabloid
	WIDTH=16.5
	HEIGHT=10.5
fi

ps=GMT_RGBchart_$SIZE.ps
allinfo=$GMT_TMPDIR/allinfo.tmp
cpt=$GMT_TMPDIR/lookup.cpt
rects=$GMT_TMPDIR/rects.tmp
whitetags=$GMT_TMPDIR/whitetags.tmp
blacktags=$GMT_TMPDIR/blacktags.tmp
labels=$GMT_TMPDIR/labels.tmp
gmtset PS_MEDIA $SIZE PS_PAGE_ORIENTATION landscape

rectheight=0.56
W=`gmtmath -Q $WIDTH $COL DIV 0.95 MUL =`
H=`gmtmath -Q $HEIGHT $ROW DIV $rectheight MUL =`
textheight=`gmtmath -Q 1 $rectheight SUB =`
fontsize=`gmtmath -Q $HEIGHT $ROW DIV $rectheight MUL 0.6 MUL 72 MUL =`
fontsizeL=`gmtmath -Q $HEIGHT $ROW DIV $textheight MUL 0.7 MUL 72 MUL =`

# Produce $allinfo from color and name files
egrep -v "^#|grey" "${GMT_SOURCE_DIR}"/src/Colors.txt | awk -v COL=$COL -v ROW=$ROW \
	'BEGIN{col=0;row=0}{if(col==0&&row<2){col++};if ($1 == $2 && $2 == $3) {printf "%s", $1} else {printf "%s/%s/%s", $1, $2,
	$3};printf " %g %s %g %g\n",0.299*$1+0.587*$2+0.114*$3,$4,col,row;col++;if(col==COL){col=0;row++}}' > $allinfo

# Produce temp files from $allinfo
awk '{printf "%g %s %g %s\n", NR-0.5, $1, NR+0.5, $1}' $allinfo > $cpt
awk -v h=$rectheight -v W=$W -v H=$H  '{printf "%g %g %g %g %g\n",$4+0.5,$5+1-0.5*h,NR,W,H}' $allinfo > $rects
awk -v h=$rectheight -v fs=$fontsize  '{if ($2 <= 127) printf "%g %g %gp,1 %s\n",$4+0.5,$5+1-0.5*h,fs,$1}' $allinfo > $whitetags
awk -v h=$rectheight -v fs=$fontsize  '{if ($2 > 127) printf "%g %g %gp,1 %s\n",$4+0.5,$5+1-0.5*h,fs,$1}' $allinfo > $blacktags
awk -v h=$textheight -v fs=$fontsizeL '{printf "%g %g %gp,1 @#%s@#\n",$4+0.5,$5+0.6*h,fs,$3}' $allinfo > $labels

# Plot all tiles and texts
psxy -R0/$COL/0/$ROW -JX$WIDTH/-$HEIGHT -X0.25i -Y0.25i -B0 -C$cpt -Sr -W $rects -K > $ps
pstext -R -J -O -K $labels -F+f --FONT=black >> $ps
pstext -R -J -O -K $blacktags -F+f --FONT=black >> $ps
pstext -R -J -O -K $whitetags -F+f --FONT=white >> $ps

# Put logo in top left corner
scale=`gmtmath -Q $WIDTH $COL DIV 0.95 MUL 2 DIV =`
xoff=`gmtmath -Q $WIDTH $COL DIV 0.05 MUL 2 DIV =`
yoff=`gmtmath -Q $HEIGHT $ROW DIV $ROW 1 SUB MUL $scale 2 DIV SUB =`
gmtlogo $xoff $yoff $scale >> $ps

H=`gmtmath -Q $HEIGHT $ROW DIV =`
pslegend -O -Dx0/0/$WIDTH/$H/BL >> $ps <<END
L $fontsizeL 1 R Values are R/G/B. Names are case-insensitive.
L $fontsizeL 1 R Optionally, use GREY instead of GRAY.
END

gmt_remove_tmpdir
