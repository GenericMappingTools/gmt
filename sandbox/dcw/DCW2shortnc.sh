#!/bin/sh
#	$Id$
#
# Script to convert the DCW text polygons to individual netCDF files
# This was the original script for the first version where all the
# polygons became individual netcdf files.  The next version will
# create a single netCDF file instead, so this file is mostly for
# documentation.

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
DIR=/Volumes/MacNutHD3/UH/RESOURCES/DATA/misc/DCW
find $DIR -name '*.dat' -print | sed -n -e 's/\.dat//gp '> t.lis
rm -f DCW.log
while read prefix; do
	item=`basename $prefix`
	echon "$item : Converting ..."
	n=`gmt_nrecords $prefix.dat`
	minmax -fg -C $prefix.dat  > BB.txt
	west=`awk '{print $1}' BB.txt`
	east=`awk '{print $2}' BB.txt`
	xrange=`awk '{print $2-$1}' BB.txt`
	south=`awk '{print $3}' BB.txt`
	north=`awk '{print $4}' BB.txt`
	yrange=`awk '{print $4-$3}' BB.txt`
	xfact=`gmtmath -Q 65535 $xrange DIV =`
	yfact=`gmtmath -Q 65535 $yrange DIV =`
	echo "$item: west = $west east = $east xrange = $xrange xfact = $xfact south = $south north = $north yrange = $yrange yfact = $yfact" >> DCW.log
	gmtmath -fig $prefix.dat -C0 $west SUB 360 ADD 360 MOD $xrange DIV 65534 MUL RINT -C1 $south SUB $yrange DIV 65534 MUL RINT -Ca = tmp.txt
	echon "Build CDL..."
	cat <<- EOF > this.cdl
	netcdf DCW {    // DCW netCDF specification in CDL
		dimensions:
			time = $n;
		variables:
			ushort lon(time);
	  		lon:valid_range = 0, 65535;
			lon:units = "0-65535";
			lon:min = $west;
			lon:max = $east;
			lon:scale  = $xfact;
			ushort lat(time);
			lat:valid_range = 0, 65535;
			lat:units = "0-65535";
			lat:min = $south;
			lat:max = $north;
			lat:scale  = $yfact;
		data:
	EOF
	awk -f xformat.awk name="lon" N=$n tmp.txt >> this.cdl
	awk -f yformat.awk name="lat" N=$n tmp.txt >> this.cdl
	cat <<- EOF >> this.cdl
	}
	EOF
	echon "Generate netCDF..."
#	ncgen -lc this.cdl > this.c
#	ncgen -b -o this.nc -k1 -x this.cdl
	echon "Deflate netCDF..."
#	ncdeflate.sh -d 9 -v this.nc
	ncgen -b -o $prefix.nc -k3 -x this.cdl
	ncdeflate.sh -d 9 $prefix.nc
	echo ""
done < t.lis
