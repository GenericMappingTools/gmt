#!/bin/sh
#	$Id$
#
# Script to convert the 319 DCW text polygons to a single nc3 netCDF file.
# We use ncdeflate.sh to build a compressed nc4 version as well
# We create variables with the <code>_ prefix, e.g. NO_lon, NO_lat for
# the Norway polygons.  For the state boundries (e.g., Texas) we create
# a prefix like USTX_, USHI_, etc.  Each polygon is scaled to fit in a short
# integer so there are attributes with scales and bounding box for each
# country.
#
# Paul Wessel, May 2013.

. gmt_shell_functions.sh

if [ x`echo -n` = x ]; then     # echo -n works
        echon()
        {
                echo -n "$*" 
        }
elif [ x`echo -e` = x ]; then   # echo -e works
        echon()
        {
                echo -e "$*"'\c'
        }
else                            # echo with escapes better work
        echon()
        {
                echo "$*"'\c'
        }
fi

# The xformat.awk and yformat.awk are helper scripts to properly format
# the data sections of the CDL file.

cat << EOF > xformat.awk
{
	if (NR == 1) {
		k = 100
		end = ""
	}
	else if (NR == 2) {
		printf "\t%s = 65535", name
		k = 1
	}
	else if (NR == N) {
		end=";"
	}
	if (k == 0) {
		if (\$1 == ">")
			printf "\t65535%s", end
		else
			printf "\t%s%s", \$1, end
		k++;
	}
	else if (k < 10) {
		if (\$1 == ">")
			printf ", 65535%s", end
		else
			printf ", %s%s", \$1, end
		k++
	}
	if (k == 10 && NR < N) {
		printf ",\n"
		k = 0;
	}
}
END {
	printf "\n"
}
EOF
cat << EOF > yformat.awk
{
	if (NR == 1) {
		k = 100
		end = ""
	}
	else if (NR == 2) {
		printf "\t%s = 65535", name
		k = 1
	}
	else if (NR == N) {
		end=";"
	}
	if (k == 0) {
		if (\$1 == ">")
			printf "\t65535%s", end
		else
			printf "\t%s%s", \$2, end
		k++;
	}
	else if (k < 10) {
		if (\$1 == ">")
			printf ", 65535%s", end
		else
			printf ", %s%s", \$2, end
		k++
	}
	if (k == 10 && NR < N) {
		printf ",\n"
		k = 0;
	}
}
END {
	printf "\n"
}
EOF
DIR=/Volumes/MacNutHD3/UH/RESOURCES/DATA/misc/DCW	# On macnut
DIR=/Users/pwessel/UH/RESOURCES/DATA/misc/DCW		# On MacAttack
find $DIR -name '*.dat' -print | sed -n -e 's/\.dat//gp '> t.lis
# macnut: The 50 is chosen so that we only print SP or US/TX, then remove the / to get USTX
# MacAttack: The 46 is chosen so that we only print SP or US/TX, then remove the / to get USTX
awk '{print substr($1,46)}' t.lis | sed -e 'sB/BBg' > var.lis
rm -f dcw-gmt.log
rm -f dim.cdl var.cdl data.cdl dcw-gmt.cdl
echon "Build CDL..."
echo "netcdf DCW {    // DCW netCDF specification in CDL" > dcw-gmt.cdl
cat <<- EOF > dim.cdl
	dimensions:
EOF
cat <<- EOF > var.cdl
	variables:
EOF
cat <<- EOF > data.cdl
	data:
EOF
let k=0
while read prefix; do
	let k=k+1
	item=`basename $prefix`
	echo "$item : Converting"
	n=`gmt_nrecords $prefix.dat`
	minmax -fg -C $prefix.dat > BB.txt
	west=`awk '{print $1}' BB.txt`
	east=`awk '{print $2}' BB.txt`
	xrange=`awk '{print $2-$1}' BB.txt`
	south=`awk '{print $3}' BB.txt`
	north=`awk '{print $4}' BB.txt`
	yrange=`awk '{print $4-$3}' BB.txt`
	xfact=`gmtmath -Q 65535 $xrange DIV =`
	yfact=`gmtmath -Q 65535 $yrange DIV =`
	echo "$item: west = $west east = $east xrange = $xrange xfact = $xfact south = $south north = $north yrange = $yrange yfact = $yfact" >> dcw-gmt.log
	gmtmath -fig $prefix.dat -C0 $west SUB 360 ADD 360 MOD $xrange DIV 65534 MUL RINT -C1 $south SUB $yrange DIV 65534 MUL RINT -Ca = tmp.txt
	VAR=`sed -n ${k}p var.lis`
cat << EOF >> dim.cdl
		${VAR}_length = $n;
EOF
cat << EOF >> var.cdl
		ushort ${VAR}_lon(${VAR}_length);
  		${VAR}_lon:valid_range = 0, 65535;
		${VAR}_lon:units = "0-65535";
		${VAR}_lon:min = $west;
		${VAR}_lon:max = $east;
		${VAR}_lon:scale  = $xfact;
		ushort ${VAR}_lat(${VAR}_length);
		${VAR}_lat:valid_range = 0, 65535;
		${VAR}_lat:units = "0-65535";
		${VAR}_lat:min = $south;
		${VAR}_lat:max = $north;
		${VAR}_lat:scale  = $yfact;
EOF
	awk -f xformat.awk name="${VAR}_lon" N=$n tmp.txt >> data.cdl
	awk -f yformat.awk name="${VAR}_lat" N=$n tmp.txt >> data.cdl
done < t.lis
cat dim.cdl >> dcw-gmt.cdl
cat var.cdl >> dcw-gmt.cdl
cat data.cdl >> dcw-gmt.cdl
cat <<- EOF >> dcw-gmt.cdl
}
EOF
echon "Generate netCDF..."
#ncgen -lc dcw-gmt.cdl > dcw-gmt.c
ncgen -b -o dcw-gmt.nc -x dcw-gmt.cdl
echo ""
rm -f *.cdl xformat.awk yformat.awk t.lis var.lis BB.txt tmp.txt
