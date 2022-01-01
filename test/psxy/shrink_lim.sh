#!/usr/bin/env bash
# Exploring the terminal limit on vector shrinking
#
gmt begin shrink_lim
    gmt set PROJ_LENGTH_UNIT inch
    gmt math -N3 -T0/2/0.1 0 -C2 90 ADD -o0:2,0 = t.txt
    gmt subplot begin 3x2 -R-0.1/2.1/-0.1/2.1 -Fs2.2i -Bafg1 -A+gwhite
        gmt subplot set -Aoriginal
        gmt plot t.txt -Sv12p+e+h0 -Gblack -W1p
        gmt subplot set -A"norm = +n1"
        gmt plot t.txt -Sv12p+e+n1/0+h0 -Gblack -W1p
        gmt subplot set -A"norm = +n1/0.5"
        gmt plot t.txt -Sv12p+e+n1/0.5+h0 -Gblack -W1p
        gmt subplot set -A"norm = +n1/0.3"
        gmt plot t.txt -Sv12p+e+n1/0.3+h0 -Gblack -W1p
        gmt subplot set -A"norm = +n1/0.2"
        gmt plot t.txt -Sv12p+e+n1/0.2+h0 -Gblack -W1p
        gmt subplot set -A"norm = +n1/0.1"
        gmt plot t.txt -Sv12p+e+n1/0.1+h0 -Gblack -W1p
    gmt subplot end
gmt end show
