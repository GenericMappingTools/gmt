#!/bin/sh
#	$Id$
#
# Just a helper script to format the controb websites that has lists of 3-column tables of images.

# This value depends on the filename length.  Experiemnt until ls -x gives 3 columns

COLUMNS=120
export COLUMNS
cd Cetaceans
ls -x *.png | sed -e 's/.png//g' > ../tmp.lis
cd ..
while read a b c; do
	cat <<- EOF
	<tr>
	<th colspan=1>$a.def</th>
	<th colspan=1>$b.def</th>
	<th colspan=1>$c.def</th>
	</tr>
	<tr>
	<td><img src="Cetaceans/$a.png" WIDTH="200" HEIGHT="200"></td>
	<td><img src="Cetaceans/$b.png" WIDTH="200" HEIGHT="200"></td>
	<td><img src="Cetaceans/$c.png" WIDTH="200" HEIGHT="200"></td>
	</tr>
	EOF
done < tmp.lis
rm -f tmp.lis
