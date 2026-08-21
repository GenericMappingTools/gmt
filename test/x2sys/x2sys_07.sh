#!/usr/bin/env bash
#
# Testing crossings near a pole.

ps=x2sys_07.ps

# Create data lines: A goes up meridian 135 to N pole and down on 315
echo "# A1:" > A1.txt
echo "# A2:" > A2.txt
echo "# B:" > B.txt
gmt math -T-18/18/1 90 T ABS NEG ADD -C0 SIGN 90 MUL 225 ADD = >> A1.txt
gmt math -T-18/18/1 90 T ABS NEG ADD -C0 SIGN 90 MUL 135 ADD = >> A2.txt
# B spirals towards the N pole but not hit it
gmt math -T0/1800/2 T 1800 DIV 14.5 MUL 75 ADD -C0 360 MOD = >> B.txt
export X2SYS_HOME=$(pwd)
gmt x2sys_init TEST -Dgeo -Etxt -F -G -Rg
gmt x2sys_cross -TTEST A1.txt A2.txt B.txt -Qe > AB.txt
gmt pscoast -R0/360/70/90 -JA0/90/7i -P -Glightgray -Baf -K -Dh -Xc > $ps
gmt psxy -R -J A[12].txt -W1p,blue -O -K >> $ps
gmt psxy -R -J B.txt -W1p,green -O -K >> $ps
gmt psxy AB.txt -R -J -O -Sc5p -Wthin,red >> $ps
