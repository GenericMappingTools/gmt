#!/bin/bash
#	$Id: cvsclean.sh,v 1.10 2011-03-15 02:06:31 guru Exp $

# BSD and Solaris commands are incompatible, use GNU versions if available
RM=$(which grm || which rm)
GREP=$(which ggrep || which grep)
AWK=$(which gawk || which awk)

# List all backup files
find . -name "*~" -o -name ".*~" -o -name "*.bak" -o -name ".*.bak" -o -name ".#*" -o -name ".*.swp" -o -name ".DS_Store" > $$.lis 

# List all the files in .cvsignore
for x in `find . -name .cvsignore` ; do
    $AWK '{printf "%s/%s\n","'`dirname $x`'",$0}' $x >>$$.lis
done

# Remove the exceptions from the list
# These are non-cvs files, we nevertheless want to keep
# The result of grep is redirected to rm command
xargs echo "ls -d -1 2>/dev/null" < $$.lis | sh | $GREP -v -e "guru/gmtguru.macros$" -e "share/coast$" | xargs $RM -rvf

# Delete the killfile
rm -f $$.lis
