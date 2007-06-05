#!/bin/sh
#       $Id: getbox.sh,v 1.2 2007-06-05 15:44:51 remko Exp $
#
# Expects -Joption and distance in km from map center
range=`(echo -$2 -$2; echo $2 $2) | mapproject $1 -R0/360/-90/90 -I -Fk -C`
echo $range | awk '{printf "-R%f/%f/%f/%fr\n",$1,$2,$3,$4}'
