#!/bin/bash
#	$Id: GMT_eckert4.sh,v 1.9 2011-06-10 23:29:27 guru Exp $
#
. ./functions.sh

pscoast -Rg -JKf4.5i -Bg30/g15 -Dc -A10000 -Wthinnest -Givory -Sbisque3 -P > GMT_eckert4.ps
