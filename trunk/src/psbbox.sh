#!/bin/sh
#	$Id: psbbox.sh,v 1.1 2009-05-21 14:55:17 remko Exp $
# NAME
#    psbbox.sh - Replace BoundingBox line in PostScript files by "real" BoundingBox
#
# SYNOPSIS
#    psbbox.sh file ...
#
# DESCRIPTION
#    This program replaces the BoundingBox line in all PostScript files
#    specified on the command line by a BoundingBox determined by the bbox
#    modules of Ghostscript.
#
# KNOWN LIMITATIONS
#    Works only for single-page PostScript files. Other limitations are
#    the limitations of the Ghostscript bbox module.
#
#    EPS Ghostscript 7.07 occasionally produces no BoundingBox at all.
#    Try using AFPL Ghostscipt 8.00 or later instead.
#
# AUTHOR
#    psbbox.s was created by Remko Scharroo on 7 October 2003.
#

for file in $*
do
   bbox=`gs -q -dNOPAUSE -dBATCH -r720 -sDEVICE=bbox $file 2>&1 | grep '%%BoundingBox'`
   if [ $? == 0 ] ; then
      tmpfile=`mktemp /tmp/psbbox.XXXXXX`
      cp -f $file $tmpfile
      sed 's/%%BoundingBox: .*$/'"$bbox/" $tmpfile > $file
      rm -f $tmpfile
   else
      echo "$0: Could not establish BoundingBox; $file unchanged"
   fi
done
