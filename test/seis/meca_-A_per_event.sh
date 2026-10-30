#!/usr/bin/env bash
#
# Check that the alternate location of a beachball (meca -A, coupe -D) only
# applies to the event that gives it.  An event without newX newY (or without
# dx dy for +o) that follows one with them must be plotted at its own location,
# not at the alternate location (or offset) of the previous event.  So each
# beachball must be centered at the same place whether its event is plotted
# alone or together with the others.

# Extract the centers of the beachball outlines, in plot order
centers () {
	grep ' Sc$' $1 | awk '{print $2, $3}'
}

# 1. meca -A with alternate coordinates (newX newY) given only for the first event
cat << EOF > xy.txt
0 0 10 0 45 90 5 3 3 Moved
5 5 10 0 45 -90 5
EOF
# 2. meca -A+o with offsets (dx dy) given only for the first event
cat << EOF > off.txt
0 0 10 0 45 90 5 1c 1c Moved
5 5 10 0 45 -90 5
EOF
# 3. coupe -D+o with offsets given only for the first event
cat << EOF > coupe.txt
128.4 11 20 0 45 90 5 1c 1c Moved
129.6 11 80 0 45 -90 5
EOF

rm -f fail
for file in xy off; do
	if [ $file = xy ]; then A="-A+p0.5p"; else A="-A+o+p0.5p"; fi
	gmt psmeca $file.txt -R-2/8/-2/8 -JX6c -Sa1c $A -P > all.ps
	centers all.ps > a.txt
	rm -f b.txt
	for n in 1 2; do
		sed -n ${n}p $file.txt | gmt psmeca -R-2/8/-2/8 -JX6c -Sa1c $A -P > one.ps
		centers one.ps >> b.txt
	done
	diff a.txt b.txt --strip-trailing-cr >> fail
done

gmt pscoupe coupe.txt -R0/250/0/100 -JX10c/-4c -Sa0.6c -Aa128/11/130/11/90/60/0/100+f -D+o+p0.5p -Q -P > all.ps
centers all.ps > a.txt
rm -f b.txt
for n in 1 2; do
	sed -n ${n}p coupe.txt | gmt pscoupe -R0/250/0/100 -JX10c/-4c -Sa0.6c -Aa128/11/130/11/90/60/0/100+f -D+o+p0.5p -Q -P > one.ps
	centers one.ps >> b.txt
done
diff a.txt b.txt --strip-trailing-cr >> fail
