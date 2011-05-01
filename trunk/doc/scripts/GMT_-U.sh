#!/bin/bash
#	$Id: GMT_-U.sh,v 1.4 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh

psxy -R0/3/0/0.1 -Jx1i -P -U"optional command string or text here" /dev/null > GMT_-U.ps
