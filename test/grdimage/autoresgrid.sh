#!/usr/bin/env bash
# Ensure that the automatic detection of suitable grid resolution works
# The -R -J and DPU below will request the earth_relief_05m_p tiles.

ps=autoresgrid.ps
gmt grdimage @earth_relief -Rd -JG13.0550/47.8095/15c+z12000+v60 --GMT_GRAPHICS_DPU=80c > $ps
