#!/usr/bin/env bash
#
# Plots a page of all 555 unique named colors
# Usage: GMT_RGBchart.sh <size>
# where <size> is the page size. Use either: ledger, a4, or letter
# This produces the file GMT_RGBchart_<size>.ps

SIZE=$1
COL=16
ROW=35

if [ "X"$SIZE"" = "Xletter" ] ; then
	WIDTH=10.5
	HEIGHT=8.0
	ORIENT=landscape
elif [ "X"$SIZE"" = "Xa4" ] ; then
	WIDTH=11.2
	HEIGHT=7.8
else
	SIZE=tabloid
	WIDTH=16.5
	HEIGHT=10.5
fi

ps=GMT_RGBchart_$SIZE.ps
allinfo=allinfo.tmp
cpt=lookup.cpt
rects=rects.tmp
whitetags=whitetags.tmp
blacktags=blacktags.tmp
labels=labels.tmp
gmt set PS_MEDIA $SIZE PS_PAGE_ORIENTATION landscape

rectheight=0.56
W=$(gmt math -Q $WIDTH $COL DIV 0.95 MUL =)
H=$(gmt math -Q $HEIGHT $ROW DIV $rectheight MUL =)
textheight=$(gmt math -Q 1 $rectheight SUB =)
fontsize=$(gmt math -Q $HEIGHT $ROW DIV $rectheight MUL 0.6 MUL 72 MUL =)
fontsizeL=$(gmt math -Q $HEIGHT $ROW DIV $textheight MUL 0.7 MUL 72 MUL =)

# Produce $allinfo from color and name files
paste ${GMT_SOURCE_DIR}/src/gmt_color_rgb.h ${GMT_SOURCE_DIR}/src/gmt_colornames.h | tr '{,}"\r' ' ' > Colors.txt
egrep -v "^#|grey" Colors.txt | $AWK -v COL=$COL -v ROW=$ROW \
	'BEGIN{col=0;row=0}{if(col==0&&row<2){col++};if ($1 == $2 && $2 == $3) {printf "%s", $1} else {printf "%s/%s/%s", $1, $2,
	$3};printf " %g %s %g %g\n",0.299*$1+0.587*$2+0.114*$3,$4,col,row;col++;if(col==COL){col=0;row++}}' > $allinfo

# Produce temp files from $allinfo
$AWK '{printf "%g %s %g %s\n", NR-0.5, $1, NR+0.5, $1}' $allinfo > $cpt
$AWK -v h=$rectheight -v W=$W -v H=$H  '{printf "%g %g %g %g %g\n",$4+0.5,$5+1-0.5*h,NR,W,H}' $allinfo > $rects
$AWK -v h=$rectheight -v fs=$fontsize  '{if ($2 <= 127) printf "%g %g %gp,1 %s\n",$4+0.5,$5+1-0.5*h,fs,$1}' $allinfo > $whitetags
$AWK -v h=$rectheight -v fs=$fontsize  '{if ($2 > 127) printf "%g %g %gp,1 %s\n",$4+0.5,$5+1-0.5*h,fs,$1}' $allinfo > $blacktags
$AWK -v h=$textheight -v fs=$fontsizeL '{printf "%g %g %gp,1 @#%s@#\n",$4+0.5,$5+0.6*h,fs,$3}' $allinfo > $labels

# Plot all tiles and texts
gmt psxy -R0/$COL/0/$ROW -JX$WIDTH/-$HEIGHT -X0.25i -Y0.25i -B0 -C$cpt -Sri -W $rects -K > $ps
gmt pstext -R -J -O -K $labels -F+f --FONT=black >> $ps
gmt pstext -R -J -O -K $blacktags -F+f --FONT=black >> $ps
gmt pstext -R -J -O -K $whitetags -F+f --FONT=white >> $ps

# Put logo in top left corner
gmt logo -R -J -O -K -Dg0.5/1+jMC+w$W >> $ps

height=$(gmt math -Q $HEIGHT $ROW DIV =)
gmt pslegend -O -R -J -DjBR+w$WIDTH >> $ps <<END
L ${fontsizeL}p,Helvetica-Bold R Values are R/G/B. Names are case-insensitive.
L ${fontsizeL}p,Helvetica-Bold R Optionally, use GREY instead of GRAY.
END
