#!/bin/bash
#	$Id$
#
#	Makes the octal code charts in Appendix F for ISO and Standard
#
# To show both ISOLatin1+ and Standard+ on the same figure we must
# make them separately then rasterize them and then plot those images
# since we cannot have two different character sets active in one PS.
# This script assumes GMT_App_F_stand+.sh and GMT_App_F_iso+.sh have
# both been run previously.

gmt ps2raster -Tg -E300 -P -A -D. "${src:-.}"/GMT_App_F_stand+.ps
gmt ps2raster -Tg -E300 -P -A -D. "${src:-.}"/GMT_App_F_iso+.ps
gmt psimage GMT_App_F_stand+.png -E300 -P -K > GMT_App_F_stand+_iso+.ps
gmt psimage GMT_App_F_iso+.png -E300 -O -X3.2i >> GMT_App_F_stand+_iso+.ps
