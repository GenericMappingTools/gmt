#!/bin/sh
#	$Id: GMT_coverlogo.sh,v 1.4 2005-02-04 16:19:46 pwessel Exp $
#
# Creates the cover page GMT logo
#
#	Logo is 5.276" wide and 2.622" high and origin is lower left
#

dpi=`../../bin/gmtget DOTS_PR_INCH`
../../bin/gmtset GRID_PEN_PRIMARY 0.25p DOTS_PR_INCH 1200
psxy -R0/1/0/1 -Jx1i -P -K -X0 -Y0 /dev/null > GMT_coverlogo.ps
../../bin/gmtlogo 0 0 2.580645 >> GMT_coverlogo.ps
cat << EOF >> GMT_coverlogo.ps
%%Trailer
%%BoundingBox: 0 0 372 186
% Reset translations and scale and call showpage
S 0 showpage

end
EOF
gmtset DOTS_PR_INCH $dpi FRAME_PEN 1.2p
