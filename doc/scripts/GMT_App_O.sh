#!/bin/sh
#	$Id: GMT_App_O.sh,v 1.1 2004-06-11 03:02:58 pwessel Exp $
#
#	Makes the inserts for Appendix O (labeled lines)
#

gmtset FRAME_WIDTH 0.04i PLOT_DEGREE_FORMAT ddd:mm:ssF ANNOT_FONT_SIZE_PRIMARY +9p
grdcut $GMTHOME/examples/ex01/osu91a1f_16.grd -R50/160/-15/15 -Gtmp.grd

# Distance algorithm
grdcontour tmp.grd -JM5i -P -B20f10WSne -C10 -A20 -Gd1.5i -S10 > GMT_APP_O_1.ps
gv  GMT_APP_O_1.ps &

# Number algorithm
grdcontour tmp.grd -JM5i -P -B20f10WSne -C10 -A20 -Gn1 -S10 > GMT_APP_O_2.ps
gv  GMT_APP_O_2.ps &

# fixed algorithm: Debug this!
cat << EOF > fix.d
77	9
55	-11.5
140	3
EOF
grdcontour tmp.grd -JM5i -P -B20f10WSne -C10 -A20 -Gffix.d/0.5i -S10 > GMT_APP_O_3.ps
gv  GMT_APP_O_3.ps &

# Straight line algorithm
grdcontour tmp.grd -JM5i -P -B20f10WSne -C10 -A20 -GlZ-/Z+ -S10 > GMT_APP_O_4.ps
gv  GMT_APP_O_4.ps &


# Complex line algorithm
grdcontour tmp.grd -JM5i -P -B20f10WSne -C10 -A20 -Gxcross.d -S10 > GMT_APP_O_4.ps
gv  GMT_APP_O_4.ps &

