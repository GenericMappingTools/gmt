#!/bin/bash
#	$Id: GMT_-U.sh,v 1.2 2011-02-28 00:58:00 remko Exp $
#
. functions.sh

psxy -R0/3/0/0.1 -Jx1 -P -U"optional command string or text here" /dev/null > GMT_-U.ps
