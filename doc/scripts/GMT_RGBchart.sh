#!/bin/bash
#	$Id: GMT_RGBchart.sh,v 1.13 2011-06-09 17:24:52 remko Exp $
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

ps=$PWD/GMT_RGBchart_$SIZE.ps
gmtset PS_MEDIA $SIZE PS_PAGE_ORIENTATION landscape

rectheight=0.56
W=`gmtmath -Q $WIDTH $COL DIV 0.95 MUL =`
H=`gmtmath -Q $HEIGHT $ROW DIV $rectheight MUL =`
textheight=`gmtmath -Q 1 $rectheight SUB =`
fontsize=`gmtmath -Q $HEIGHT $ROW DIV $rectheight MUL 0.6 MUL 72 MUL =`
fontsizeL=`gmtmath -Q $HEIGHT $ROW DIV $textheight MUL 0.7 MUL 72 MUL =`

GMTHOME=$(cd ../..; pwd)
cd $GMT_TMPDIR

# Produce allinfo.tmp from color and name files
egrep -v "^#|grey" $GMTHOME/src/Colors.txt | awk -v COL=$COL -v ROW=$ROW \
	'BEGIN{col=0;row=0}{if(col==0&&row<2){col++};if ($1 == $2 && $2 == $3) {printf "%s", $1} else {printf "%s/%s/%s", $1, $2, $3};printf " %g %s %g %g\n",0.299*$1+0.587*$2+0.114*$3,$4,col,row;col++;if(col==COL){col=0;row++}}' > allinfo.tmp

# Produce temp files from allinfo.tmp
awk '{printf "%g %s %g %s\n", NR-0.5, $1, NR+0.5, $1}' allinfo.tmp > lookup.cpt
awk -v h=$rectheight -v W=$W -v H=$H  '{printf "%g %g %g %g %g\n",$4+0.5,$5+1-0.5*h,NR,W,H}' allinfo.tmp > rects.tmp
awk -v h=$rectheight -v fs=$fontsize  '{if ($2 <= 127) printf "%g %g %gp,1 %s\n",$4+0.5,$5+1-0.5*h,fs,$1}' allinfo.tmp > whitetags.tmp
awk -v h=$rectheight -v fs=$fontsize  '{if ($2 > 127) printf "%g %g %gp,1 %s\n",$4+0.5,$5+1-0.5*h,fs,$1}' allinfo.tmp > blacktags.tmp
awk -v h=$textheight -v fs=$fontsizeL '{printf "%g %g %gp,1 @#%s@#\n",$4+0.5,$5+0.6*h,fs,$3}' allinfo.tmp > labels.tmp

# Plot all tiles and texts
psxy -R0/$COL/0/$ROW -JX$WIDTH/-$HEIGHT -X0.25i -Y0.25i -B0 -Clookup.cpt -Sr -W rects.tmp -K > $ps
pstext -R -J -O -K labels.tmp -F+f --FONT=black >> $ps
pstext -R -J -O -K blacktags.tmp -F+f --FONT=black >> $ps
pstext -R -J -O -K whitetags.tmp -F+f --FONT=white >> $ps

# Put logo in top left corner
scale=`gmtmath -Q $WIDTH $COL DIV 0.95 MUL 2 DIV =`
xoff=`gmtmath -Q $WIDTH $COL DIV 0.05 MUL 2 DIV =`
yoff=`gmtmath -Q $HEIGHT $ROW DIV $ROW 1 SUB MUL $scale 2 DIV SUB =`
gmtlogo $xoff $yoff $scale >> $ps

H=`gmtmath -Q $HEIGHT $ROW DIV =`
pslegend -R -J -O -Dx0/0/$WIDTH/$H/BL >> $ps <<END
L $fontsizeL 1 BR Values are R/G/B. Names are case-insensitive.
L $fontsizeL 1 BR Optionally, use GREY instead of GRAY.
END

cd `dirname $ps`
gmt_remove_tmpdir
