#!/bin/sh
#	$Id: gmt_hitmap.sh,v 1.2 2004-09-29 21:23:39 pwessel Exp $
#
# Make the GMT ftpsite hitmap PNG image and the hitmap that goes with it.
# The files created are gmt_hitmap.png and gmt_hitmap.map
#
dpi=72
yoff=1
dia=0.2
#-------------------------------------------------------------------------------------------------
# ADD NEW MIRRORS HERE.
# Remember: Only ONE TAB between fields, otherwise the next awk gets confused.
cat << EOF > mirrors.d
146:58	-36:0	ftp://life.csu.edu.au/pub/gmt	CT	ALBURY - AUSTRALIA
10:44	59:55	ftp://ftp.geologi.uio.no/pub/gmt	CB	OSLO - NORWAY
141:10	43:02	ftp://ftp.eos.hokudai.ac.jp/pub/gmt	CB	SAPPORO - JAPAN
-77:0	38:52	ftp://falcon.grdl.noaa.gov/pub/gmt	CT	SILVER SPRING - USA
-46:40	-23:32	ftp://ftp.iag.usp.br/pub/gmt	CT	S\303O PAULO - BRAZIL
16:22	48:12	ftp://gd.tuwien.ac.at	CT	VIENNA - AUSTRIA
-122:22	47:40	ftp://ftp.iris.washington.edu	CB	SEATTLE - USA
EOF
echo "-157:59	21:55	ftp://gmt.soest.hawaii.edu/pub/gmt	CT	HONOLULU - USA" > master.d
#-------------------------------------------------------------------------------------------------
if [ $# -eq 1 ]; then
	gush=0
else
	gush=1
fi
if [ $gush ]; then
	echo "gmt_hitmap.sh: Preparing the web page hitmap"
fi
awk -F'	' '{printf "%s\t%s\t9\t0\t1\t%s\t%s\n", $1, $2, $4, $5}' mirrors.d > ftp.d
awk -F'	' '{printf "%s\t%s\t12\t0\t1\t%s\t%s\n", $1, $2, $4, $5}' master.d > us.d
pscoast -Rd -JN180/6i -Slightblue -Gbrown -Dc -A1000 -B0g30/0g15 -K -P -Y$yoff}i --PAPER_MEDIA=letter+ --CHAR_ENCODING=ISOLatin1+ > gmt_hitmap.ps
# Draw spokes from Hawaii to each site
i=1
while read lon lat rest; do
	cut -f1,2 master.d > t
	echo "$lon $lat" >> t
	psxy -R -J -O -K -W2p,darkgreen t -A >> gmt_hitmap.ps
done < ftp.d
# Place yellow and red circles
psxy -R -J -O -K -Sc$dia}i -Gyellow -Wthin ftp.d >> gmt_hitmap.ps
psxy -R -J -O -K -Sc${dia}i -Gred -Wthin us.d >> gmt_hitmap.ps
# Add site labels
pstext -R -J -O -K ftp.d -W255O -N -Dj0.15i/0.175i >> gmt_hitmap.ps
pstext -R -J -O -K us.d -W0O -Gwhite -N -Dj0.15i/0.2i >> gmt_hitmap.ps
# Draw the legend
pslegend -R0/5/0/1 -Jx1i -O -K -D3/0/2.1i/0.6i/CT -Gtan -Y-0.1i -F2p -L1.25 << EOF >> gmt_hitmap.ps
S 0.2i c 0.2i red 0.25p 0.45i GMT Master Site
S 0.2i c 0.2i yellow 0.25p 0.45i GMT Mirror Site
EOF
psxy -R -J -O /dev/null >> gmt_hitmap.ps
convert -density $dpi -crop 0x0 gmt_hitmap.ps gmt_hitmap.png
if [ $gush ]; then
	xv gmt_hitmap.png &
fi
H=`echo 180 90 | mapproject -JN180/6i -Rd | cut -f2`
width=`gmtmath -Q 6 $dpi MUL =`
height=`gmtmath -Q $H $dpi MUL =`
rad=`gmtmath -Q $dia $dpi MUL 2 DIV =`
cat  mirrors.d master.d | mapproject -JN180/$width -Rd | awk '{printf "circle\t%s\t %d,%d %d,%d\n", $3, int($1+0.5), int('$height'-$2+0.5), int($1+'$rad'+0.5), int('$height'-$2+0.5)}' > gmt_hitmap.map
rm -f mirrors.d master.d ftp.d us.d gmt_hitmap.ps
