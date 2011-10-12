#!/bin/sh
# $Id$
# Looks for files with \r in them
if [ $# -eq 1 ]; then
	long=1
else
	long=0
fi
find . -name '*.[ch]' -print >| /tmp/$$.lis
find . -name '*.tex' -print >> /tmp/$$.lis
find . -name '*.txt' -print >> /tmp/$$.lis
find . -name '*.in' -print >> /tmp/$$.lis
find . -name '*.conf' -print >> /tmp/$$.lis
find . -name '*.info' -print >> /tmp/$$.lis
find . -name '*.def' -print >> /tmp/$$.lis
find . -name '*akefile' -print >> /tmp/$$.lis
find . -name 'README*' -print >> /tmp/$$.lis
find share -name '*.cpt' -print >> /tmp/$$.lis
find share -name '*.d' -print >> /tmp/$$.lis
find share -name '*.ps' -print >> /tmp/$$.lis
find share -name '*.html' -print >> /tmp/$$.lis
find share -name '*.[135]' -print >> /tmp/$$.lis
find share -name '*.sh' -print >> /tmp/$$.lis
find share -name '*.csh' -print >> /tmp/$$.lis
find share -name '*.bat' -print >> /tmp/$$.lis
find share -name gmtfile_paths -print >> /tmp/$$.lis
let n_DOS=0
let n_unix=0
while read name; do
	od -c $name | grep '\\r' >| /tmp/$$.junk
	if [ -s /tmp/$$.junk ]; then
		printf "%-60s: DOS\n" $name
		let n_DOS=n_DOS+1
	else
		if [ $long -eq 1 ]; then
			printf "%-60s: Unix\n" $name
		fi
		let n_unix=n_unix+1
	fi
done < /tmp/$$.lis
rm -f /tmp/$$.*
let n_tot=n_DOS+n_unix
printf "DOS files found: %d Unix files found: %d\n" $n_DOS $n_unix
