#!/bin/sh
#
#	GMT Example 24  $Id: job24.sh,v 1.1 2004-04-23 22:50:42 pwessel Exp $
#
# Purpose:	
# GMT progs:	
# Unix progs:	
#
psbasemap -R0/6.5/0/9 -Jx1i -B0 -P -U"Example 24 in Cookbook" > example_24.ps
rm -f .gmtcommands4 .gmtdefaults4
