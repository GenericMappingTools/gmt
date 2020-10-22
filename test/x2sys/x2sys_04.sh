#!/usr/bin/env bash
#
# This is original Figure 4 script from
# Wessel, P. (2010), Tools for analyzing intersecting tracks: the x2sys package,
# Computers & Geosciences, 36, 348-354.
# Here used as a test for the x2sys suite.

ps=x2sys_04.ps

OLDX=$X2SYS_HOME
export X2SYS_HOME=.

rm -rf TEST
ln -fs "${GMT_SRCDIR:-.}"/bad "${GMT_SRCDIR:-.}"/data .
(cd bad; ls *.xyg) > bad.lis
(cd data; ls *.xyg) > data.lis

gmt x2sys_init TEST -Dgeoz -Exyg -F -G -R180/185/0/5
echo "bad" >> TEST/TEST_paths.txt
gmt x2sys_cross -TTEST =data.lis -Qe -Ia -V > COE_clean.txt
gmt x2sys_cross -TTEST =bad.lis -Qe -Ia -V > COE_orig.txt
gmt x2sys_list -TTEST COE_orig.txt -Cz -Fndc > COE_use.txt
gmt x2sys_solve -TTEST COE_use.txt -Cz -Ed > corrections.txt
R=181/185/0/3
gmt makecpt -Crainbow -T-80/80 > faa.cpt

# Grid the corrected data
gmt x2sys_datalist -TTEST -Lcorrections.txt =bad.lis -Flon,lat,z | gmt blockmean -R$R -I1m | gmt surface -R$R -I1m -Gss_gridded_fix1.nc -T0.25
gmt grdgradient ss_gridded_fix1.nc -Ne0.75 -A65 -fg -Gss_gridded_fix1_int.nc
gmt grdimage ss_gridded_fix1.nc -Iss_gridded_fix1_int.nc -Ei -JM5.5i -P -K -Cfaa.cpt -B1 -BWsne -X1.75i -Y5.75i --MAP_FRAME_WIDTH=3p --FORMAT_GEO_MAP=dddF > $ps

# Obtain adjustments and grid the corrected and adjusted data
gmt x2sys_report -TTEST -Cz COE_orig.txt -Lcorrections.txt -A > /dev/null
gmt x2sys_datalist -TTEST -A -Lcorrections.txt =bad.lis -Flon,lat,z | gmt blockmean -R$R -I1m | gmt surface -R$R -I1m -Gss_gridded_fix2.nc -T0.25
gmt grdgradient ss_gridded_fix2.nc -Ne0.75 -A65 -fg -Gss_gridded_fix2_int.nc
gmt grdimage ss_gridded_fix2.nc -Iss_gridded_fix2_int.nc -Ei -JM5.5i -O -Cfaa.cpt -B1 -BWSne -Y-4.5i --MAP_FRAME_WIDTH=3p --FORMAT_GEO_MAP=dddF >> $ps

if [ ! "X$OLDX" = "X" ]; then	# Reset prior setting
	export X2SYS_HOME=$OLDX
fi
