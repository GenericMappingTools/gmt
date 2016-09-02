#!/bin/bash
#	$Id$
#
# Description: 

PS=pssac_E.ps

gmt set PS_MEDIA 21cx50c
# -En
gmt pssac *.z -R200/1600/-0.9/3.9 -JX15c/5c -Bx200+l'T(s)' -By1+lN -BWSen -W1p,blue -En -M1.5c -P -K > $PS
# -En1
gmt pssac *.z -R200/1600/0.1/4.9 -JX15c/5c -Bx200 -By1+lN1 -BWsen -W1p,blue -En1 -M1.5c -K -O -Y6c >> $PS
# -Ea
gmt pssac *.z -R200/1600/10/75 -JX15c/5c -Bx200 -By20+lAZ -BWsen -W1p,blue -Ea -M1.5c -K -O -Y6c >> $PS
# -Eb
gmt pssac *.z -R200/1600/200/290 -JX15c/5c -Bx200 -By20+lBAZ -BWsen -W1p,blue -Eb -M1.5c -K -O -Y6c >> $PS
# -Ek
gmt pssac *.z -R200/1600/1500/5000 -JX15c/5c -Bx200 -By1000+lkm -BWsen -W1p,blue -Ek -M1.5c -K -O -Y6c >> $PS
# -Ed
gmt pssac *.z -R200/1600/15/40 -JX15c/5c -Bx200 -By5+lDegree -BWsen -W1p,blue -Ed -M1.5c -K -O -Y6c >> $PS
# -Eu0: ray parameter in s/radian
gmt pssac *.z -R200/1600/420/750 -JX15c/5c -Bx200 -By50+lRayPar -BWsen -W1p,blue -Eu0 -M1.5c -K -O -Y6c >> $PS
gmt psxy -J -R -O -T >> $PS
