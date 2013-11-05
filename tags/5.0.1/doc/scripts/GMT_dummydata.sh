#!/bin/bash
#
#	$Id$
#
#	This script makes the dummy data sets needed in Section 5.1
. ./functions.sh

gmtmath -T0/100/1  T SQRT = sqrt.d
gmtmath -T0/100/10 T SQRT = sqrt.d10
