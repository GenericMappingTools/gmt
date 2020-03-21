#!/usr/bin/env bash
#
#
# Purpose:      Test all PSL functions at least once
# GMT progs:    libpostscriptlight, psldemo
# Unix progs:   -
#
ps=psldemo.ps
export PSL_SHAREDIR=$(gmt --show-sharedir)
psldemo > $ps
