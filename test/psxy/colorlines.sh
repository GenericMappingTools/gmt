#!/usr/bin/env bash
#
# Set colors applied to lines via cpt and -Z in header

ps=colorlines.ps

echo "> -Z0" > lines.txt
gmt math -T0/5/1 T 0.2 MUL = >> lines.txt
echo "> -Z1" >> lines.txt
gmt math -T0/5/1 T 0.2 MUL 1 ADD = >> lines.txt
echo "> -Z2" >> lines.txt
gmt math -T0/5/1 T 0.2 MUL 2 ADD = >> lines.txt
echo "> -Z3" >> lines.txt
gmt math -T0/5/1 T 0.2 MUL 3 ADD = >> lines.txt
echo "> -Z4" >> lines.txt
gmt math -T0/5/1 T 0.2 MUL 4 ADD = >> lines.txt
echo "> -Z5" >> lines.txt
gmt math -T0/5/1 T 0.2 MUL 5 ADD = >> lines.txt
echo "> -Z6" >> lines.txt
gmt math -T0/5/1 T 0.2 MUL 6 ADD = >> lines.txt
echo "> -Z67" >> lines.txt
gmt math -T0/5/1 T 0.2 MUL 7 ADD = >> lines.txt
gmt makecpt -Cjet -T0/7/1 > t.cpt
gmt psxy -R-1/6/-1/8 -JX6i/9i -P -B1g1 -BWSne -Xc lines.txt -Ct.cpt -W5p+cl > $ps
