#!/bin/bash
#	$Id: GMT_-U.sh,v 1.3 2011-03-15 02:06:29 guru Exp $
#
. functions.sh

psxy -R0/3/0/0.1 -Jx1i -P -U"optional command string or text here" /dev/null > GMT_-U.ps
