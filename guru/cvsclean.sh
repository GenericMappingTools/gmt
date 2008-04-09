#!/bin/sh
#	$Id: cvsclean.sh,v 1.8 2008-04-09 18:16:48 remko Exp $

# List all backup files
find . -name "*~" -o -name ".*~" -o -name "*.bak" -o -name ".*.bak" -o -name ".#*" -o -name ".*.swp" -o -name ".DS_Store" > $$.lis 
# List all the files in .cvsignore
for x in `find . -name .cvsignore` ; do
    $AWK '{printf "%s/%s\n","'`dirname $x`'",$0}' $x >>$$.lis
done

# Remove the exceptions from the list
# These are non-cvs files, we nevertheless want to keep
# The result of grep is redirected to rm command
xargs echo "ls -d -1 2>/dev/null" < $$.lis | sh | grep -v -e "guru/gmtguru.macros$" -e "share/coast$" | xargs rm -rvf

# Delete the killfile
rm -f $$.lis
