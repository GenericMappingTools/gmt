#!/usr/bin/env bash
#
# Plots a page of all 663 unique named colors

COL=16
ROW=35

WIDTH=16.5
HEIGHT=10.5
allinfo=allinfo.tmp
cpt=lookup.cpt
rects=rects.tmp
whitetags=whitetags.tmp
blacktags=blacktags.tmp
labels=labels.tmp

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

gmt begin GMT_RGBchart
	gmt set PS_MEDIA tabloid PS_PAGE_ORIENTATION landscape

	# Plot all tiles and texts
	gmt plot -R0/$COL/0/$ROW -JX$WIDTH/-$HEIGHT -X0.25i -Y0.25i -B0 -C$cpt -Sri -W $rects
	gmt text $labels -F+f --FONT=black
	gmt text $blacktags -F+f --FONT=black
	gmt text $whitetags -F+f --FONT=white

	# Put logo in top left corner
	gmt logo -Dg0.5/1+jMC+w$W

	height=$(gmt math -Q $HEIGHT $ROW DIV =)
	gmt legend -DjBR+w$WIDTH <<END
L ${fontsizeL}p,Helvetica-Bold R Values are R/G/B. Names are case-insensitive.
L ${fontsizeL}p,Helvetica-Bold R Optionally, use GREY instead of GRAY.
END
gmt end show
