#!/bin/bash
#	$Id$

# BSD and Solaris commands are incompatible, use GNU versions if available
RM=$(which grm || which rm)
GREP=$(which ggrep || which grep)
AWK=$(which gawk || which awk)
list1=`mktemp -t gmt.XXXXXXXX`
list2=`mktemp -t gmt.XXXXXXXX`

# List all files and directories in SVN
find . -name "*.svn*" | sed -e "s:/.svn/text-base::" -e "s:/.svn/prop-base::" -e "s:.svn-base::" -e "s:/.svn::" | \
	sort | uniq > $list1

# List all local files and directories (except .svn directories)
find . -not -name "*.svn*" | $GREP -v .svn | sort | uniq > $list2

# Remove the files that are in list2 but not in list 1
# These are non-svn files, we nevertheless want to keep
# The result of grep is redirected to rm command
diff $list1 $list2 | $GREP ">" | cut -c3- | $GREP -v -e "./guru/gmtguru.macros" -e "./share/coast" | xargs $RM -rvf

# Delete the temporary lists
rm -f $list1 $list2
