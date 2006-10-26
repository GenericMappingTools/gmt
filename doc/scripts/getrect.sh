#!/bin/sh
#       $Id: getrect.sh,v 1.1 2006-10-26 18:03:07 remko Exp $
#
# Expects -Joption and xmin xmax ymin ymax in km relative to map center
(echo $2 $4; echo $3 $4; echo $3 $5; echo $2 $5) | mapproject $1 -R0/360/-90/90 -I -Fk -C
