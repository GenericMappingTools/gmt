#!/usr/bin/env bash
#
# Check that the intensities read from the input file by meca -I
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

gmt psmeca meca.txt -R0/8/0/2 -JX12c/3c -Sa1c -Gred -I -P > all.ps
fills all.ps > a.txt
head -n 1 meca.txt | gmt psmeca -R0/8/0/2 -JX12c/3c -Sa1c -Gred -I -P > one.ps
fills one.ps > one.txt
cat one.txt one.txt one.txt one.txt > b.txt

diff a.txt b.txt --strip-trailing-cr > fail
