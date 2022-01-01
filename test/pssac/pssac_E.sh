#!/usr/bin/env bash
#
# Description:

ps=pssac_E.ps
SACFILEs="${src:-.}"/*.z

gmt set PS_MEDIA 21cx50c
# -En
gmt pssac $SACFILEs -R200/1600/-0.9/3.9 -JX15c/5c -Bx200+l'T(s)' -By1+lN -BWSen -W1p,blue -En -M1.5c -P -K > $ps
# -En1
gmt pssac $SACFILEs -R200/1600/0.1/4.9 -J -Bx200 -By1+lN1 -BWsen -W1p,blue -En1 -M1.5c -K -O -Y6c >> $ps
# -Ea
gmt pssac $SACFILEs -R200/1600/10/75 -J -Bx200 -By20+lAZ -BWsen -W1p,blue -Ea -M1.5c -K -O -Y6c >> $ps
# -Eb
gmt pssac $SACFILEs -R200/1600/200/290 -J -Bx200 -By20+lBAZ -BWsen -W1p,blue -Eb -M1.5c -K -O -Y6c >> $ps
# -Ek
gmt pssac $SACFILEs -R200/1600/1500/5000 -J -Bx200 -By1000+lkm -BWsen -W1p,blue -Ek -M1.5c -K -O -Y6c >> $ps
# -Ed
gmt pssac $SACFILEs -R200/1600/15/40 -J -Bx200 -By5+lDegree -BWsen -W1p,blue -Ed -M1.5c -K -O -Y6c >> $ps
# -Eu0: ray parameter in s/radian
gmt pssac $SACFILEs -R200/1600/420/750 -J -Bx200 -By50+lRayPar -BWsen -W1p,blue -Eu0 -M1.5c -O -Y6c >> $ps
