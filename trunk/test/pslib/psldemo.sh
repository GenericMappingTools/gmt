#!/bin/bash
#
# $Id$
#
# Purpose:      Test all PSL functions at least once
# GMT progs:    libpslib, psldemo
# Unix progs:   -
#
. ../functions.sh
header "Test pslib capabilities"
ps=psldemo.ps
psldemo > $ps
pscmp
