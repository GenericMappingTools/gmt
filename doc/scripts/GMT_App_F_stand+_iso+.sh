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

bash "${src:-.}"/func_F_stand+.sh > stand+.ps
bash "${src:-.}"/func_F_iso+.sh > iso+.ps
gmt psconvert -Tg -E600 -P -Z -A -D. stand+.ps iso+.ps
gmt psimage stand+.png -Dx0/0+r600 -P -K > GMT_App_F_stand+_iso+.ps
gmt psimage iso+.png -Dx0/0+r600 -O -X3.2i >> GMT_App_F_stand+_iso+.ps
