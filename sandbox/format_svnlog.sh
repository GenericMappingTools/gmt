#!/bin/bash
#	$Id$
# Run svn log to get a ChangeLog-type format in chronological order
# from the given date to today, e.g.
#
# format_log.sh 2013-11-08
now=`date -u +"%Y-%m-%d"`
svn log -r {$now}:{2013-11-08} | egrep -v '\-\-\-|^$' | awk '{if ($2 =="|" && $4 == "|") {printf "\n%s\t%s\n", $5, $3} else {printf "\t* filename.ext:\t\t%s\n", $0}}'
