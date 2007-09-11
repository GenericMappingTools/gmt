#!/bin/sh
#	$Id: plot_errbars.sh,v 1.2 2007-09-11 22:56:12 remko Exp $
#
# Plot error bars and test [+|-]<epen>

echo -n "$0: Test psxy error bar colors:				"

ps=plot_errbars.ps
cat << EOF > $$
1	1	1	1
2	2	2	1
3	3	3	1
4	4	4	1
5	5	5	1
EOF
makecpt -Crainbow -T0/6/1 > $$.cpt
psxy -R0/6/0/6 -JX3i -P -B0 -Sc0.2i -C$$.cpt -W0.25p -X1i -Y2i $$ -Ex/2p,red -K > $ps
psxy -R -J -O -B0 -Sc0.2i -C$$.cpt -W0.25p -X3.25i $$ -Ey/-1p -K >> $ps
psxy -R -J -O -B0 -Sc0.2i -C$$.cpt -W+5p -X-3.25i -Y3.5i $$ -Ey/+1p -K >> $ps
psxy -R -J -O -B0 -Sc0.2i -C$$.cpt -W0.25p,red -X3.25i $$ -Ex/-1p >> $ps
rm -f $$*
compare -density 100 -metric PSNR {,orig/}$ps plot_errbars_diff.png > log 2>&1
grep inf log > fail
if [ ! -s fail ]; then
        echo "[FAIL]"
	echo $0 >> ../fail_count.d
else
        echo "[PASS]"
        rm -f fail plot_errbars_diff.png log
fi
