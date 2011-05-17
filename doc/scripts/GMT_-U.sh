#!/bin/bash
#	$Id: GMT_-U.sh,v 1.5 2011-05-17 00:23:50 guru Exp $
#
. ./functions.sh

psxy -R0/3/0/0.1 -Jx1i -P -U"optional command string or text here" -T > GMT_-U.ps
