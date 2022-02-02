#!/usr/bin/env bash
# Ensure that the automatic detection of suitable grid resolution works
# The -R -J and DPU below will request two earth_relief_05m_p tiles.
# DVC_TEST

ps=autoresgrid.ps
gmt grdimage @earth_relief -R-20/20/0/40 -JM10c --GMT_GRAPHICS_DPU=40c -P > $ps
