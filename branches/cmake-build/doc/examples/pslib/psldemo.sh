#!/bin/bash
#
# $Id$
#
# Purpose:      Test all PSL functions at least once
# GMT progs:    libpslib
# Unix progs:   -
#
. ../functions.sh
ps=../psldemo.ps
psldemo > $ps
