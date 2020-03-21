#!/usr/bin/env bash
#
# Testing gmt legend capabilities for tables with colors

gmt begin GMT_legend
gmt set FONT_ANNOT_PRIMARY 12p  FONT_LABEL 12p

cat <<EOF > table.txt
#G 0.04i
H 24 Times-Roman Eight Largest Cities in North America
D 1p
N 6 22 16 20 20 8 8
V 0.25p
S 0.15i c 0.1i snow1 -
L - - C City Name
L - - C Country
L - - C Population
L - - C Climate
L - - C WC?
L - - C OL?
D 0 1p
F lightgreen
S 0.15i c 0.1i red 0.25p
L - - R Mexico City
L - - R Mexico
L - - R 8,851,080
L - - R Tropical
L - - C Y
L - - C Y
F -
S 0.15i c 0.1i orange 0.25p
L - - R New York City
L - - R USA
L - - R 8,405,837
L - - R Tempered
L - - C Y
L - - C N
S 0.15i c 0.1i yellow 0.25p
L - - R Los Angeles
L - - R USA
L - - R 3,904,657
L - - R Subtropical
L - - C Y
L - - C Y
F lightblue
S 0.15i c 0.1i green 0.25p
L - - R Toronto
L - - R Canada
L - - R 2,795,060
L - - R Tempered
L - - C N
L - - C N
F -
S 0.15i c 0.1i blue 0.25p
L - - R Chicago
L - - R USA
L - - R 2,714,856
L - - R Tempered
L - - C Y
L - - C N
S 0.15i c 0.1i cyan 0.25p
L - - R Houston
L - - R USA
L - - R 2,714,856
L - - R subtropical
L - - C N
L - - C N
F lightred
S 0.15i c 0.1i magenta 0.25p
L - - R Havana
L - - R Cuba
L - - R 2,106,146
L - - R Tropical
L - - C N
L - - C N
F lightblue
S 0.15i c 0.1i white 0.25p
L - - R Montreal
L - - R Canada
L - - R 1,649,519
L - - R Tempered
L - - C N
L - - C Y
D 1p
V 1p
F -
N 1
L 9 4 R Information from Wikipedia
G 0.05i
T Many of these cities have hosted World Cup Soccer (WC) and some
T have hosted the Olympics (OL).  The rest is just some basic information
T about each city, such as climate and population.  Of course, this is all
T an excuse to demonstrate variable-width tables and row coloring.
#G 0.1i
EOF
cat << EOF > t.cpt
1	red
2	orange
3	yellow
4	green
5	blue
6	cyan
7	magenta
8	white
EOF
gmt legend -Dx0/0+w5.6i+jBL+l1.2 -C0.05i -F+p+gsnow1 -B0 table.txt --FONT_ANNOT_PRIMARY=12p  --FONT_LABEL=12p
rm -f table.txt t.cpt
gmt end show
