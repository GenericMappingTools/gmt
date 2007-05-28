#!/bin/sh
#	$Id: seis.sh,v 1.4 2007-05-28 19:40:30 pwessel Exp $
#
echo -n "GMT: Test psscale and makecpt combinations:		"
makecpt -T-6/6/1 -Cseis -D > tmp.cpt
psscale -Ctmp.cpt -D00/04/8/0.5 -K           > seis.ps
psscale -Ctmp.cpt -D00/13/8/0.5 -O -K -I -N100 >> seis.ps
makecpt -T-6/6/1 -Cseis -D | grep -v ^0 | sed 's/^1/0/' > tmp.cpt
psscale -Ctmp.cpt -D04/04/8/0.5 -O -K       >> seis.ps
psscale -Ctmp.cpt -D04/13/8/0.5 -O -K -I -N100 >> seis.ps
psscale -Ctmp.cpt -D08/04/8/0.5 -O -K -L    >> seis.ps
psscale -Ctmp.cpt -D08/13/8/0.5 -O -K -L -I -N100 >> seis.ps
psscale -Ctmp.cpt -D12/04/8/0.5 -O -K -L0.1    >> seis.ps
psscale -Ctmp.cpt -D12/13/8/0.5 -O -K -L0.1 -I -N100 >> seis.ps
makecpt -T-6/6/1 -Cseis -D -Z > tmp.cpt
psscale -Ctmp.cpt -D16/04/8/0.5 -O -K       >> seis.ps
psscale -Ctmp.cpt -D16/13/8/0.5 -O -K -I -N100 >> seis.ps
makecpt -T-6/6/1 -Cseis -D -Z | grep -v ^0 | sed 's/^1/0/' > tmp.cpt
psscale -Ctmp.cpt -D20/04/8/0.5 -O -K       >> seis.ps
psscale -Ctmp.cpt -D20/13/8/0.5 -O    -I -N100 >> seis.ps

rm -f tmp.cpt .gmtcommands4

compare -density 100 -metric PSNR seis_orig.ps seis.ps seis_diff.png > log
grep inf log > fail
if [ ! -s fail ]; then
        echo "[FAILED]"
else
        echo "[OK"]
        rm -f fail seis_diff.png log
fi
