#!/bin/sh
#
#	$Id: GMT_dummydata.sh,v 1.1 2001-03-21 04:10:21 pwessel Exp $
#
#	This script makes the dummy data sets needed in Section 5.1

gmtmath -T0/100/1  T SQRT = sqrt.d
gmtmath -T0/100/10 T SQRT = sqrt.d10
