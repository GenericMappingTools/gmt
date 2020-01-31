#!/usr/bin/env bash
#
# Compute FAA, VGG, and geoid over synthetic seamount
ps=fields.ps
order=2
dx=1
gmt set GMT_FFT kiss
function Ugly {
	# sub-function for integral I (z, b, c)
	z=$1
	b=$2
	c=$3
	beta2=$(gmt math -Q 1.0 $b $b MUL ADD =)
	beta=$(gmt math -Q $beta2 SQRT =)
	beta3=$(gmt math -Q $beta2 $beta MUL =)
	P1=$(gmt math -Q $z $z MUL 2 $b MUL $c MUL $z MUL $c $c MUL ADD $beta2 DIV ADD SQRT $beta DIV =)
	P2=$(gmt math -Q $z $b $c MUL $beta2 ADD DIV $z $z MUL 2 $b MUL $c MUL $z MUL $c $c MUL ADD $beta2 DIV ADD SQRT ADD LOG =)
	gmt math -Q $z $P1 SUB $b $c MUL $P2 MUL $beta3 DIV ADD =
#	v = z - sqrt (z * z + (2.0 * b * c * z  + c * c) / beta2) / beta + ...
#		b * c * log (z + b * c / beta2 + sqrt (z * z + (2.0 * b * c * z  + c * c) / beta2)) / beta3;
#	  = z - P1 + b *c * P2 / beta3
}

# 2 panels of topo and grav, with top profile of admittance & coherence
# NOT FINISHED
# 1. Create a bathymetry data set with one circular truncated seamount
#    as in Fig 3. of Marks & Smith, 2007 [GRL], with R_base = 35 km,
# R_top = 10 km, height = 3751 m, depth = -5084 m, density d_rho = 2800-1030
# = 1670 kg/m^3, so the flattening is 10/25 = 0.4.
echo "0	0	25	3751" | gmt grdseamount -R-256/256/-256/256 -I$dx -r -C -Gsmt.nc -F0.4 -Z-5084
# BL Plot the bathymetry
gmt makecpt -Crainbow -T-5100/-1000 > t.cpt
gmt grdimage smt.nc -R-100/100/-100/100 -JX3i -P -Bag -BWSne -Ct.cpt -K > $ps
gmt grdtrack -Gsmt.nc -ELM/RM+d > smt.trk
gmt psxy -R -J -O -K -W5p,white smt.trk >> $ps
gmt psxy -R -J -O -K -W1p smt.trk >> $ps
echo "-100 100 BATHYMETRY" | gmt pstext -R -J -O -K -F+jTL+f14p -Dj0.1i -Gwhite -C+tO >> $ps
# 2. Compute the VGG anomaly
gmt gravfft smt.nc+uk -D1670 -Nf+a -Fv -E$order -Gvgg.nc
# BR plot the VGG anomaly
gmt makecpt -Crainbow -T-50/250 > t.cpt
gmt grdimage vgg.nc -R-100/100/-100/100 -JX3i -O -Bag -BwSne -Ct.cpt -K -X3.5i >> $ps
gmt grdtrack -Gvgg.nc -ELM/RM+d > vgg.trk
gmt psxy -R -J -O -K -W5p,white vgg.trk >> $ps
gmt psxy -R -J -O -K -W1p,blue vgg.trk >> $ps
echo "-100 100 VGG" | gmt pstext -R -J -O -K -F+jTL+f14p -Dj0.1i -Gwhite -C+tO >> $ps
# 3. Compute the FAA anomaly
gmt gravfft smt.nc+uk -D1670 -Nf+a -Ff -E$order -Gfaa.nc
# Compute the exact analytical result for peak amplitude at center
r0=10000
z0=$(gmt math -Q 5084 3751 SUB =)
r1=35000
z1=5084
rho=$(gmt math -Q 2800 1030 SUB =)
b=$(gmt math -Q $r1 $r0 SUB $z1 $z0 SUB DIV =)
c=$(gmt math -Q $r0 $b $z0 MUL SUB =)
I1=$(Ugly $z1 $b $c)
I2=$(Ugly $z0 $b $c)
gmax=$(gmt math -Q 2 PI MUL 6.673e-6 MUL $rho MUL $I1 $I2 SUB MUL =)
echo "Max FAA should be $gmax mGal"
# ML plot the FAA anomaly
gmt makecpt -Crainbow -T-50/250 > t.cpt
gmt grdimage faa.nc -R-100/100/-100/100 -JX3i -O -Bag -BWsne -Ct.cpt -K -X-3.5i -Y3.25i >> $ps
gmt grdtrack -Gfaa.nc -ELM/RM+d > faa.trk
gmt psxy -R -J -O -K -W5p,white faa.trk >> $ps
gmt psxy -R -J -O -K -W1p,red faa.trk >> $ps
echo "-100 100 FAA" | gmt pstext -R -J -O -K -F+jTL+f14p -Dj0.1i -Gwhite -C+tO >> $ps
# 4. Compute the geoid anomaly
gmt gravfft smt.nc+uk -D1670 -Nf+a -Fg -E$order -Ggeoid.nc
# MR plot the VGG anomaly
gmt makecpt -Crainbow -T0/5 > t.cpt
gmt grdimage geoid.nc -R-100/100/-100/100 -JX3i -O -Bag -Bwsne -Ct.cpt -K -X3.5i >> $ps
gmt grdtrack -Ggeoid.nc -ELM/RM+d > geoid.trk
gmt psxy -R -J -O -K -W5p,white geoid.trk >> $ps
gmt psxy -R -J -O -K -W1p,orange geoid.trk >> $ps
echo "-100 100 GEOID" | gmt pstext -R -J -O -K -F+jTL+f14p -Dj0.1i -Gwhite -C+tO >> $ps
# 5 Plot crossections of bathy and faa crossections
# TL plot the bathy and faa canomaly
gmt psxy -R-100/100/-5100/1000 -JX3i/2.5i -O -K -W1p -i0,3 smt.trk -Baf -BWsN -X-3.5i -Y3.2i >> $ps
echo "-100 1000 TOPO" | gmt pstext -R -J -O -K -F+jTL+f12p -Dj0.1i/0.15i >> $ps
gmt psxy -R-100/100/-50/250 -J -O -K -W1p,red -i0,3 faa.trk -Bafg1000 -BENs >> $ps
gmt psxy -R -J -O -K -W0.5p,- << EOF >> $ps
-100	$gmax
+100	$gmax
EOF
echo "100 250 FAA" | gmt pstext -R -J -O -K -F+jTR+f12p,Helvetica,red -Dj0.1i/0.15i >> $ps
# Add VGG and geoid crossections
# TRL plot the VGG and geoid anomaly
gmt psxy -R-100/100/-50/250 -JX3i/2.5i -O -K -W1p,blue -i0,3 vgg.trk -Bafg1000 -BwsN -X3.5i >> $ps
echo "-100 250 VGG" | gmt pstext -R -J -O -K -F+jTL+f12p,Helvetica,blue -Dj0.1i >> $ps
gmt psxy -R-100/100/0/4 -J -O -K -W1p,orange -i0,3 geoid.trk -Baf -BE >> $ps
echo "100 4 GEOID" | gmt pstext -R -J -O -K -F+jTR+f12p,Helvetica,orange -Dj0.1i >> $ps
gmt psxy -R -J -O -T >> $ps
