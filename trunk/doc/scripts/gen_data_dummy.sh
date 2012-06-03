#!/bin/bash
#
# $Id$
# This script makes the dummy data sets needed in GMT_linear.sh GMT_log.sh GMT_pow.sh
#
gmtmath -T0/100/1  T SQRT = sqrt.d
gmtmath -T0/100/10 T SQRT = sqrt.d10
