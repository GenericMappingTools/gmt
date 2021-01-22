#!/usr/bin/env bash
#
# Demonstrate failure to clip ellipses against periodic TM boundary in y.
# See https://forum.generic-mapping-tools.org/t/how-to-avoid-the-funky-look-of-jt-and-se/1224
ps=tissot_TM.ps
echo 30 -60 2000k | gmt psxy -R0/360/-80/80 -JT-15/15/16c -Bafg -SE- -Gred@50 -Wred -P > $ps
