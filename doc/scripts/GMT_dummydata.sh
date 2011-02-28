#!/bin/bash
#
#	$Id: GMT_dummydata.sh,v 1.2 2011-02-28 00:58:01 remko Exp $
#
#	This script makes the dummy data sets needed in Section 5.1
. functions.sh

gmtmath -T0/100/1  T SQRT = sqrt.d
gmtmath -T0/100/10 T SQRT = sqrt.d10
