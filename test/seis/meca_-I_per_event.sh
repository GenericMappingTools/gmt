#!/usr/bin/env bash
#
# Check that the intensities read from the input file by meca -I and coupe -I
# (without -C) are applied to the -G fill of each event, not on top of the fill
# already illuminated for the previous event.  Four events with the same
# intensity must all get the same fill as one event plotted alone.

# Extract the fill colors set for the beachballs, in plot order
fills () {
	grep -o '{[0-9.]* [0-9.]* [0-9.]* C} FS' $1 | sed -e 's/[{}]//g' -e 's/ C FS//'
}

cat << EOF > meca.txt
1 1 10 0 45 90 5 -0.5
3 1 10 0 45 90 5 -0.5
5 1 10 0 45 90 5 -0.5
7 1 10 0 45 90 5 -0.5
EOF
cat << EOF > coupe.txt
128.4 11 20 0 45 90 5 -0.5
128.8 11 40 0 45 90 5 -0.5
129.2 11 60 0 45 90 5 -0.5
129.6 11 80 0 45 90 5 -0.5
EOF

gmt psmeca meca.txt -R0/8/0/2 -JX12c/3c -Sa1c -Gred -I -P > all.ps
fills all.ps > a.txt
head -n 1 meca.txt | gmt psmeca -R0/8/0/2 -JX12c/3c -Sa1c -Gred -I -P > one.ps
fills one.ps > one.txt
cat one.txt one.txt one.txt one.txt > b.txt

gmt pscoupe coupe.txt -R0/250/0/100 -JX10c/-4c -Sa0.6c -Aa128/11/130/11/90/60/0/100+f -Gred -I -Q -P > all.ps
fills all.ps > c.txt
head -n 1 coupe.txt | gmt pscoupe -R0/250/0/100 -JX10c/-4c -Sa0.6c -Aa128/11/130/11/90/60/0/100+f -Gred -I -Q -P > one.ps
fills one.ps > one.txt
cat one.txt one.txt one.txt one.txt > d.txt

diff a.txt b.txt --strip-trailing-cr > fail
diff c.txt d.txt --strip-trailing-cr >> fail
