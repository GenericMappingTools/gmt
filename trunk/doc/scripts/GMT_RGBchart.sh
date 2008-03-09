#!/bin/sh
#	$Id: GMT_RGBchart.sh,v 1.2 2008-03-09 21:22:30 remko Exp $
#
# Plots a page of all 555 unique named colors
# Usage: GMT_RGBchart.sh <size>
# where <size> is the page size. Use either: ledger, a4, or letter
# This produces the file GMT_RGBchart_<size>.ps

. gmt_shell_functions.sh

gmt_init_tmpdir

SIZE=$1
if [ $SIZE == letter ] ; then
	COL=11
	ROW=51
	WIDTH=8.0
	HEIGHT=10.5
elif [ $SIZE == a4 ] ; then
	COL=11
	ROW=51
	WIDTH=7.8
	HEIGHT=11.2
else
	SIZE=ledger
	COL=15
	ROW=37
	WIDTH=16.5
	HEIGHT=10.5
fi

ps=$PWD/GMT_RGBchart_$SIZE.eps
gmtset DOTS_PR_INCH 600 PAPER_MEDIA $SIZE PAGE_ORIENTATION portrait

rectheight=0.7
W=`gmtmath -Q $WIDTH $COL DIV 0.95 MUL =`
H=`gmtmath -Q $HEIGHT $ROW DIV $rectheight MUL 0.8 MUL =`
textheight=`gmtmath -Q 1 $rectheight SUB =`
fontsize=`gmtmath -Q $HEIGHT $ROW DIV $rectheight MUL 0.5 MUL 72 MUL =`
fontsizeL=`gmtmath -Q $HEIGHT $ROW DIV $textheight MUL 72 MUL =`

cd $GMT_TMPDIR

cat $GMTHOME/src/gmt_color_rgb.h | tr -d '{},' | \
	awk '{if ($1 == $2 && $2 == $3) {printf "%s", $1} else {printf "%s/%s/%s", $1, $2, $3};printf " %g", 0.299*$1+0.587*$2+0.114*$3;if (($1==0||$1==64||$1==127||$1==191||$1==255)&&($2==0||$2==64||$2==127||$2==191||$2==255)&&($3==0||$3==64||$3==127||$3==191||$3==255)){printf " *\n"}else{printf "\n"}}' > colors.tmp
cat $GMTHOME/src/gmt_colornames.h | tr -d '",' > names.tmp
paste {name,color}s.tmp | grep -vi grey | awk -v COL=$COL -v textheight=$textheight -v rectheight=$rectheight \
	'BEGIN{col=0;row=0}{printf "%g %g %s %s %s\n",col,row,$1$4,$2,$3;col++;if(col==COL){col=0;row++}}' > allinfo.tmp

awk '{printf "%g %s %g %s\n", NR-0.5, $4, NR+0.5, $4}' allinfo.tmp > lookup.cpt
awk -v h=$rectheight -v W=$W -v H=$H  '{printf "%g %g %g %g %g\n",$1+0.5,$2+1-0.5*h,NR,W,H}' allinfo.tmp > rects.tmp
awk -v h=$rectheight -v fs=$fontsize  '{if ($5 <= 127) printf "%g %g %g 0 1 CM %s\n",$1+0.5,$2+1-0.5*h,fs,$4}' allinfo.tmp > whitetags.tmp
awk -v h=$rectheight -v fs=$fontsize  '{if ($5 > 127) printf "%g %g %g 0 1 CM %s\n",$1+0.5,$2+1-0.5*h,fs,$4}' allinfo.tmp > blacktags.tmp
awk -v h=$textheight -v fs=$fontsizeL '{printf "%g %g %g 0 1 CM @#%s@#\n",$1+0.5,$2+0.5*h,fs,$3}' allinfo.tmp > labels.tmp

psxy -R0/$COL/0/$ROW -JX$WIDTH/-$HEIGHT -X0.25i -Y0.25i -B0 -Clookup.cpt -Sr -W0.25p rects.tmp -K -P > $ps
pstext -R -J -O -K labels.tmp >> $ps
pstext -R -J -O -K blacktags.tmp -Gblack >> $ps
pstext -R -J -O -K whitetags.tmp -Gwhite >> $ps

if [ $COL == 11 ] ; then
	echo $COL $ROW $fontsize 0 1 BR Optionally use GREY instead of GRAY. *WYSIWYG color. | \
	pstext -R -J -O -N -Y${fontsizeL}p -X-${fontsizeL}p >> $ps
else
	psxy -R -J -O /dev/null >> $ps
fi

cd `dirname $ps`
gmt_remove_tmpdir
