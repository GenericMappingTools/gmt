#!/bin/sh
#	$Id$
#
# Test script that exercise the various options in x2sys.
# We generate a grid and some fake tracks and sample the
# grid, then add various systematic errors to the tracks
# and finally try to solve for and undo the errors.

# 1. Make a surface grid with a Mexican hat bump in the middle

delete=0	# Set to 0 for debug where files are not removed

grdmath -R-4/4/-4/4 -I0.1 0 0 CDIST DUP DUP MUL NEG 4 DIV EXP EXCH 3 MUL COS MUL = hat.nc

# 2. Create 3 fake tracks
cat << EOF | mapproject -Gc | sample1d -Fl -T2 -I0.1 | grdtrack -Ghat.nc > trackA.xydz
-3	-3
3	3
3	1
-2	1
EOF
cat << EOF | mapproject -Gc | sample1d -Fl -T2 -I0.1 | grdtrack -Ghat.nc > trackB.xydz
3	-1
0	2
0	-2
-2	0
EOF
# THrow in a wrench by scaling the grid by 1.1 before sampling track C:
grdmath hat.nc 1.1 MUL = $$.nc
cat << EOF | mapproject -Gc | sample1d -Fl -T2 -I0.1 | grdtrack -G$$.nc > trackC.xydz
-3	-1
2	-1
2	3
-2	-1
EOF
rm -f $$.nc

# Create a *.def file and initialize the tag

cat << EOF > xydz.def
# This file applies to plain 4-column ascii files with no header record.
#
#---------------------------------------------------------------------
#ASCII		# The input file is ascii
#SKIP 0		# The number of header records to skip
#---------------------------------------------------------------------
#name	intype	NaN-proxy?	NaN-proxy	scale	offset	oformat
x	a	N		0		1	0	-
y	a	N		0		1	0	-
d	a	N		0		1	0	-
z	a	N		0		1	0	-
EOF

x2sys_init FAKE -Dxydz -V -F -R-5/5/-5/5 -I1 -Cc
rm -f xydz.def

x2sys_cross -TFAKE track[ABC].xydz -Qe -V -2 > fake_COE_orig.txt

makecpt -T-1/1/0.1 -Cpolar -Z > hat.cpt
PS=x2sys_1.ps
grdview hat.nc -Chat.cpt -JX6i -P -B1g1WSne -Qs -Wc1p -K -X1.25i > $PS
psxy -R -J -O -K trackA.xydz -W2p,red >> $PS
psxy -R -J -O -K trackB.xydz -W2p,blue >> $PS
psxy -R -J -O -K trackC.xydz -W2p,black >> $PS
psxy -R -J -O -K -M fake_COE_orig.txt -Sc0.15 -W0.5p >> $PS
head -1 trackA.xydz | psxy -R -J -O -K -Sc0.1i -Gred >> $PS
head -1 trackB.xydz | psxy -R -J -O -K -Sc0.1i -Gblue >> $PS
head -1 trackC.xydz | psxy -R -J -O -K -Sc0.1i -Gblack >> $PS
cut -f3,4 trackA.xydz | psxy -R0/16/-1/1 -JX6i/2i -B5f1/0.2:."X2SYS 1\072 3 tracks":WSne -O -K -Y6.5i -W2p,red >> $PS
cut -f3,4 trackB.xydz | psxy -R -J -O -K -W2p,blue >> $PS
cut -f3,4 trackC.xydz | psxy -R -J -O -W2p,black >> $PS
gv $PS &

#---------------------------------------------------------
# Test 1: Add some constant shifts and try to resolve them
#---------------------------------------------------------

gmtmath trackA.xydz -C3 0.2 ADD -Ca = trackAc.xydz
gmtmath trackB.xydz -C3 0.1 SUB -Ca = trackBc.xydz
gmtmath trackC.xydz -C3 0.05 ADD -Ca = trackCc.xydz

# Obtain all external crossovers

x2sys_cross -TFAKE track[ABC]c.xydz -Qe -V -2 > fake_COE_constant.txt
x2sys_list -TFAKE -Cz fake_COE_constant.txt -Fnc > COE.txt

# Make track plots for shifted tracks

PS=x2sys_2.ps
cut -f3,4 trackA.xydz | psxy -R0/16/-1.5/1.5 -JX6i/2.25i -X1.25i -P -Y7.25i -B5f1/0.2g10:."X2SYS 2\072 Constants added":WSne -K -W0.25p,red,- > $PS
cut -f3,4 trackAc.xydz | psxy -R -J -O -K -W2p,red >> $PS
cut -f3,4 trackB.xydz | psxy -R -J -O -K -W0.25p,blue,- >> $PS
cut -f3,4 trackBc.xydz | psxy -R -J -O -K -W2p,blue >> $PS
cut -f3,4 trackC.xydz | psxy -R -J -O -K -W0.25p,black,- >> $PS
cut -f3,4 trackCc.xydz | psxy -R -J -O -K -W2p,black >> $PS

cut -f3,4 trackAc.xydz | psxy -R -J -O -K -B5f1/0.2g10WSne -Y-2.25i -W1p,red >> $PS
x2sys_list -TFAKE -Cz fake_COE_constant.txt -Fdc -StrackA | psxy -R -J -O -K -Sc0.05 -Gblack >> $PS
cut -f3,4 trackBc.xydz | psxy -R -J -O -K -B5f1/0.2g10WSne -Y-2.25i -W1p,blue >> $PS
x2sys_list -TFAKE -Cz fake_COE_constant.txt -Fdc -StrackB | psxy -R -J -O -K -Sc0.05 -Gblack >> $PS
cut -f3,4 trackCc.xydz | psxy -R -J -O -K -B5f1/0.2g10WSne -Y-2.25i -W1p,black >> $PS
x2sys_list -TFAKE -Cz fake_COE_constant.txt -Fdc -StrackC | psxy -R -J -O -K -Sc0.05 -Gblack >> $PS
psxy -R -J -O /dev/null >> $PS
gv $PS &

# Solve for constants
x2sys_solve COE.txt -TFAKE -Cz -Ec -V > $X2SYS_HOME/FAKE/corr_const.lis
A=`grep trackA corr.lis | cut -f3`
B=`grep trackB corr.lis | cut -f3`
C=`grep trackC corr.lis | cut -f3`

# Correct tracks
x2sys_datalist -TFAKE -Lcorr_const.lis trackAc.xydz > trackAcc.xydz
x2sys_datalist -TFAKE -Lcorr_const.lis trackBc.xydz > trackBcc.xydz
x2sys_datalist -TFAKE -Lcorr_const.lis trackCc.xydz > trackCcc.xydz

# Make track plots for corrected tracks
x2sys_cross -TFAKE track[ABC]cc.xydz -Qe -V -2 > fake_COE_constant_corr.txt

PS=x2sys_3.ps
cut -f3,4 trackA.xydz | psxy -R0/16/-1.5/1.5 -JX6i/2.25i -X1.25i -P -Y7.25i -B5f1/0.2g10:."X2SYS 3\072 Constants resolved":WSne -K -W0.25p,red,- > $PS
cut -f3,4 trackAcc.xydz | psxy -R -J -O -K -W2p,red >> $PS
cut -f3,4 trackB.xydz | psxy -R -J -O -K -W0.25p,blue,- >> $PS
cut -f3,4 trackBcc.xydz | psxy -R -J -O -K -W2p,blue >> $PS
cut -f3,4 trackC.xydz | psxy -R -J -O -K -W0.25p,black,- >> $PS
cut -f3,4 trackCcc.xydz | psxy -R -J -O -K -W2p,black >> $PS

cut -f3,4 trackAcc.xydz | psxy -R -J -O -K -B5f1/0.2g10WSne -Y-2.25i -W1p,red >> $PS
x2sys_list -TFAKE -Cz fake_COE_constant_corr.txt -Fdc -StrackA | psxy -R -J -O -K -Sc0.05 -Gblack >> $PS
cut -f3,4 trackBcc.xydz | psxy -R -J -O -K -B5f1/0.2g10WSne -Y-2.25i -W1p,blue >> $PS
x2sys_list -TFAKE -Cz fake_COE_constant_corr.txt -Fdc -StrackB | psxy -R -J -O -K -Sc0.05 -Gblack >> $PS
cut -f3,4 trackCcc.xydz | psxy -R -J -O -K -B5f1/0.2g10WSne -Y-2.25i -W1p,black >> $PS
x2sys_list -TFAKE -Cz fake_COE_constant_corr.txt -Fdc -StrackC | psxy -R -J -O -K -Sc0.05 -Gblack >> $PS
psxy -R -J -O /dev/null >> $PS
gv $PS &
#---------------------------------------------------------
# Test 2: Add some linear drifts and try to resolve them
#---------------------------------------------------------

awk '{printf "%s\t%s\t%s\t%g\n", $1, $2, $3, $4 + 0.2 + 0.04*$3}' trackA.xydz > trackAd.xydz
awk '{printf "%s\t%s\t%s\t%g\n", $1, $2, $3, $4 - 0.1 + 0.03*$3}' trackB.xydz > trackBd.xydz
awk '{printf "%s\t%s\t%s\t%g\n", $1, $2, $3, $4 + 0.05 - 0.025*$3}' trackC.xydz > trackCd.xydz

# Obtain all external crossovers

x2sys_cross -TFAKE track[ABC]d.xydz -Qe -V -2 > fake_COE_drift.txt
x2sys_list -TFAKE -Cz fake_COE_drift.txt -Fndc > COE.txt

# Make track plots for shifted tracks

PS=x2sys_4.ps
cut -f3,4 trackA.xydz | psxy -R0/16/-1.5/1.5 -JX6i/2.25i -X1.25i -P -Y7.25i -B5f1/0.2g10:."X2SYS 4\072 Drifts added":WSne -K -W0.25p,red,- > $PS
cut -f3,4 trackAd.xydz | psxy -R -J -O -K -W2p,red >> $PS
cut -f3,4 trackB.xydz | psxy -R -J -O -K -W0.25p,blue,- >> $PS
cut -f3,4 trackBd.xydz | psxy -R -J -O -K -W2p,blue >> $PS
cut -f3,4 trackC.xydz | psxy -R -J -O -K -W0.25p,black,- >> $PS
cut -f3,4 trackCd.xydz | psxy -R -J -O -K -W2p,black >> $PS

cut -f3,4 trackAd.xydz | psxy -R -J -O -K -B5f1/0.2g10WSne -Y-2.25i -W1p,red >> $PS
x2sys_list -TFAKE -Cz fake_COE_drift.txt -Fdc -StrackA | psxy -R -J -O -K -Sc0.05 -Gblack >> $PS
cut -f3,4 trackBd.xydz | psxy -R -J -O -K -B5f1/0.2g10WSne -Y-2.25i -W1p,blue >> $PS
x2sys_list -TFAKE -Cz fake_COE_drift.txt -Fdc -StrackB | psxy -R -J -O -K -Sc0.05 -Gblack >> $PS
cut -f3,4 trackCd.xydz | psxy -R -J -O -K -B5f1/0.2g10WSne -Y-2.25i -W1p,black >> $PS
x2sys_list -TFAKE -Cz fake_COE_drift.txt -Fdc -StrackC | psxy -R -J -O -K -Sc0.05 -Gblack >> $PS
psxy -R -J -O /dev/null >> $PS
gv $PS &

# Solve for trends
x2sys_solve COE.txt -TFAKE -Cz -Ed -V > $X2SYS_HOME/FAKE/corr_trend.lis

# Correct tracks
x2sys_datalist -TFAKE -Lcorr_trend.lis trackAd.xydz > trackAdc.xydz
x2sys_datalist -TFAKE -Lcorr_trend.lis trackBd.xydz > trackBdc.xydz
x2sys_datalist -TFAKE -Lcorr_trend.lis trackCd.xydz > trackCdc.xydz

# Make track plots for corrected tracks
x2sys_cross -TFAKE track[ABC]dc.xydz -Qe -V -2 > fake_COE_drift_corr.txt

PS=x2sys_5.ps
cut -f3,4 trackA.xydz | psxy -R0/16/-1.5/1.5 -JX6i/2.25i -X1.25i -P -Y7.25i -B5f1/0.2g10:."X2SYS 5\072 Drifts resolved":WSne -K -W0.25p,red,- > $PS
cut -f3,4 trackAdc.xydz | psxy -R -J -O -K -W2p,red >> $PS
cut -f3,4 trackB.xydz | psxy -R -J -O -K -W0.25p,blue,- >> $PS
cut -f3,4 trackBdc.xydz | psxy -R -J -O -K -W2p,blue >> $PS
cut -f3,4 trackC.xydz | psxy -R -J -O -K -W0.25p,black,- >> $PS
cut -f3,4 trackCdc.xydz | psxy -R -J -O -K -W2p,black >> $PS

cut -f3,4 trackAdc.xydz | psxy -R -J -O -K -B5f1/0.2g10WSne -Y-2.25i -W1p,red >> $PS
x2sys_list -TFAKE -Cz fake_COE_drift_corr.txt -Fdc -StrackA | psxy -R -J -O -K -Sc0.05 -Gblack >> $PS
cut -f3,4 trackBdc.xydz | psxy -R -J -O -K -B5f1/0.2g10WSne -Y-2.25i -W1p,blue >> $PS
x2sys_list -TFAKE -Cz fake_COE_drift_corr.txt -Fdc -StrackB | psxy -R -J -O -K -Sc0.05 -Gblack >> $PS
cut -f3,4 trackCdc.xydz | psxy -R -J -O -K -B5f1/0.2g10WSne -Y-2.25i -W1p,black >> $PS
x2sys_list -TFAKE -Cz fake_COE_drift_corr.txt -Fdc -StrackC | psxy -R -J -O -K -Sc0.05 -Gblack >> $PS
psxy -R -J -O /dev/null >> $PS
gv $PS &
if [ $delete -eq 1 ]; then
	rm -f hat.nc hat.cpt track[ABC]*.xydz fake_COE_*.txt COE.txt corr_const.lis corr_trend.lis
fi

