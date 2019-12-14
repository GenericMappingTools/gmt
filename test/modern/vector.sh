#!/usr/bin/env bash
# Test that +p +g overrides defaults from -W -G respectively
gmt begin vector ps
echo 1 1 4 1 | gmt plot -R0/5/0/2 -Jx1i -Sv0.2i+s+b+e

echo 1 1 4 1 | gmt plot -Sv0.2i+s+b+e -W1p -Y1c
echo 1 1 4 1 | gmt plot -Sv0.2i+s+b+e+p -W1p -Y1c
# Try the deprecated +p-
echo 1 1 4 1 | gmt plot -Sv0.2i+s+b+e+p- -W1p -Y1c
echo 1 1 4 1 | gmt plot -Sv0.2i+s+b+e+p3p -W1p -Y1c

echo 1 1 4 1 | gmt plot -Sv0.2i+s+b+e -Gblack -Y1c
echo 1 1 4 1 | gmt plot -Sv0.2i+s+b+e+g -Gblack -Y1c
# Try the deprecated +g-
echo 1 1 4 1 | gmt plot -Sv0.2i+s+b+e+g- -Gblack -Y1c
echo 1 1 4 1 | gmt plot -Sv0.2i+s+b+e+gred -Gblack -Y1c

echo 1 1 4 1 | gmt plot -Sv0.2i+s+b+e -W1p -Gblack -Y1c
echo 1 1 4 1 | gmt plot -Sv0.2i+s+b+e+p+gred -W1p -Gblack -Y1c
gmt end show
