#!/bin/sh
#
#	GMT Example 21  $Id: job21.sh,v 1.1 2004-04-23 22:50:42 pwessel Exp $
#
# Purpose:	
# GMT progs:	
# Unix progs:	
#
psbasemap -R0/6.5/0/9 -Jx1i -B0 -P -U"Example 21 in Cookbook" > example_21.ps
rm -f .gmtcommands4 .gmtdefaults4
