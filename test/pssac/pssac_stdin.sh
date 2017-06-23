#!bin/bash
#	$Id$
#
# Description:

R=195/1600/14/42
J=X15c/6c
Bx=x250
By=y5
PS=pssac_stdin.ps

gmt set PS_MEDIA 21cx31c
gmt pssac "${src:-.}"/*.z -J$J -R$R -B$Bx -B$By -BWSen -Ed -M0.8i -K -P > $PS
gmt pssac input.dat -J -R -B$Bx -B$By -BWsen -Ed -M0.8i -K -O -Y7c >> $PS
gmt pssac -J -R -B$Bx -B$By -BWsen -Ed -M0.8i -K -O -Y7c >> $PS << EOF
# This is a comment, it will be skipped
ntkl.z 195 16 2p,red
nykl.z 300 20 0p,blue
onkl.z 400 30 2.2p,yellow
sdkl.z 500 35 2.2p
EOF
gmt pssac -J -R -B$Bx -B$By -BWsen -Ed -M0.8i -K -O -Y7c -h1 >> $PS << EOF
name    X  Y   pen
ntkl.z 195 16 2p,red
nykl.z 300 20 0p,blue
onkl.z 400 30 2.2p,yellow
sdkl.z 500 35 2.2p
EOF
gmt psxy -J -R -O -T >> $PS
