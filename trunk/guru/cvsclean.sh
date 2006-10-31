#!/bin/sh
#	$Id: cvsclean.sh,v 1.4 2006-10-31 18:01:10 remko Exp $

trap 'rm -f $$.*; exit 1' 1 2 3 15

# create the killfile
cat /dev/null >$$.lis

# all the files in .cvsignore
for x in `find . -name .cvsignore`
do
  $AWK '{printf "%s/%s\n","'`dirname $x`'",$0}' $x >>$$.lis
done

# remove backup files
find . -name "*~" >>$$.lis
find . -name ".*~" >>$$.lis
find . -name "*.bak" >>$$.lis
find . -name ".*.bak" >>$$.lis
find . -name ".#*" >>$$.lis

# remove the exceptions from the list
# these are non-cvs files, we nevertheless want to keep
#
# the result of grep is redirected to rm command
cat $$.lis | awk '{printf "ls -d -1 %s 2>/dev/null\n",$0}' |sh | grep -v \
-e "guru/gmtguru.macros$" \
-e "share/coast$" \
| $AWK '{printf "rm -rvf \"%s\"\n",$0}' |sh

# delete the killfile
rm $$.lis
