makecpt -T-6/6/1 -Cseis -D > tmp.cpt
psscale -Ctmp.cpt -D00/04/8/0.5 -K           > seis.ps
psscale -Ctmp.cpt -D00/13/8/0.5 -O -K -B2f1 >> seis.ps
makecpt -T-6/6/1 -Cseis -D | grep -v ^0 | sed 's/^1/0/' > tmp.cpt
psscale -Ctmp.cpt -D04/04/8/0.5 -O -K       >> seis.ps
psscale -Ctmp.cpt -D04/13/8/0.5 -O -K -B2f1 >> seis.ps
makecpt -T-6/6/1 -Cseis -D -Z > tmp.cpt
psscale -Ctmp.cpt -D08/04/8/0.5 -O -K       >> seis.ps
psscale -Ctmp.cpt -D08/13/8/0.5 -O -K -B2f1 >> seis.ps
makecpt -T-6/6/1 -Cseis -D -Z | grep -v ^0 | sed 's/^1/0/' > tmp.cpt
psscale -Ctmp.cpt -D12/04/8/0.5 -O -K       >> seis.ps
psscale -Ctmp.cpt -D12/13/8/0.5 -O    -B2f1 >> seis.ps

rm -f tmp.cpt .gmtcommands4

echo -n "Comparing seis_orig.ps and seis.ps: "
compare -density 100 -metric PSNR seis_orig.ps seis.ps seis_diff.png
