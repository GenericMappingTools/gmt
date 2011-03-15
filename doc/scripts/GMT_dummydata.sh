#!/bin/bash
#
#	$Id: GMT_dummydata.sh,v 1.3 2011-03-15 02:06:29 guru Exp $
#
#	This script makes the dummy data sets needed in Section 5.1
. functions.sh

gmtmath -T0/100/1  T SQRT = sqrt.d
gmtmath -T0/100/10 T SQRT = sqrt.d10
