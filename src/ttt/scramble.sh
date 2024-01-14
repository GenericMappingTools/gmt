#!/usr/bin/env bash
#
#	scramble.sh
#
# scramble will make the ttt code harder to figure out by substituting the
# original variable, macro, and function names with meaningless substitute
# names such as a90213 etc.  I created a plain list of all macros, variables
# and functions defined in my code (e.g., we ignore system stuff).  These
# are called ${PREFIX}_macros.lis, ${PREFIX}_variables.lis and ${PREFIX}_functions.lis
# The script then produces random new names for these entities and does
# a big substitution.  You have to watch out for bad replements in text
# etc since it is just a dumb sed job.

PREFIX=$1
OBF=$2

if [ "X$3" = "Xdebug" ]; then # Just rename include stuff
	cat << EOF > scramble.sed
s/#include "ttt.h"/#include "ttt${OBF}.h"/g
s/#include "ttt.c"/#include "ttt${OBF}.c"/g
s/#include "${PREFIX}_macro.h"/#include "${PREFIX}${OBF}_macro.h"/g
s/#include "${PREFIX}.h"/#include "${PREFIX}${OBF}.h"/g
s/#include "${PREFIX}_subs.c"/#include "${PREFIX}${OBF}_subs.c"/g
EOF
	exit
fi

# Get combined listing of macros and variables and get number of items
cat ${PREFIX}_macros.lis ${PREFIX}_variables.lis | grep -v '^#' > $$.v.lis
cat ${PREFIX}_functions.lis | grep -v '^#' > $$.f.lis
NV=`cat $$.v.lis | awk '{print $1}'`
NF=`cat $$.f.lis | awk '{print $1}'`

# Generate random numbers between two arbitrary but large integers:
gmt math -T1/10001/1 -N1 -Ca 998765 133113 RAND RINT = $$.seeds.lis

rm -f scramble.sed
# Do functions
while read name; do
	L=`gmt math -Q 1 10001 RAND RINT =`	# Get a random entry from 1-100001
	X=`sed -n ${L}p $$.seeds.lis`		# Pick the random number at this record
	grep -v $X $$.seeds.lis >> $$.d		# Remove this number (and duplicates) from list
	mv -f $$.d $$.seeds.lis
	C=`gmt math -Q 116 122 RAND =`		# Get a random ascii entry from 't'-'z'
#	echo $name $C $X | awk '{printf "s/%s/%c%d/g\n", $1, $2, $3}' >> scramble.sed
	echo $name $C $X | awk '{printf "s/%s/%c%06d/g\n", $1, $2, $3}' >> scramble.sed
done < $$.f.lis
# Do variables and macros
while read name; do
	L=`gmt math -Q 1 10001 RAND RINT =`	# Get a random entry from 1-100001
	X=`sed -n ${L}p $$.seeds.lis`		# Pick the random number at this record
	grep -v $X $$.seeds.lis >> $$.d		# Remove this number (and duplicates) from list
	mv -f $$.d $$.seeds.lis
	C=`gmt math -Q 97 115 RAND =`		# Get a random ascc entry from 'a'-'s'
#	echo $name $C $X | awk '{printf "s/%s/%c%d/g\n", $1, $2, $3}' >> scramble.sed
	echo $name $C $X | awk '{printf "s/%s/%c%06d/g\n", $1, $2, $3}' >> scramble.sed
done < $$.v.lis
rm -f $$.*
#
# Change the include statements for ttt.h, ${PREFIX}_macro.h and ${PREFIX}_subs.c to be ttt${OBF}* instead
# We leave the ${PREFIX}_arch* stuff alone
cat << EOF >> scramble.sed
s/#include "ttt.h"/#include "ttt${OBF}.h"/g
s/#include "ttt.c"/#include "ttt${OBF}.c"/g
s/#include "${PREFIX}_macro.h"/#include "${PREFIX}${OBF}_macro.h"/g
s/#include "${PREFIX}_lib.h"/#include "${PREFIX}${OBF}_lib.h"/g
s/#include "${PREFIX}.h"/#include "${PREFIX}${OBF}.h"/g
s/#include "${PREFIX}_subs.c"/#include "${PREFIX}${OBF}_subs.c"/g
EOF
