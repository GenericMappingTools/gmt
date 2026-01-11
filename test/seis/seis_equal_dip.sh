#!/usr/bin/env bash
#
# Check psmeca for cases where P and T axes have equal dips (e.g., vertical dip-slip)

gmt begin seis_equal_dip ps
# 1. Vertical Dip-Slip Fault (Dip=90, Rake=90)
# P-axis dip = 45, T-axis dip = 45
echo 2.0 4.0 10.0 0 90 90 5.0 0 0 | gmt meca -Sa2.5c -R0/10/0/6 -JM15c -B1 -Gred -W1p

# 2. Strike-Slip Fault (Dip=90, Rake=0)
# P-axis dip = 0, T-axis dip = 0
echo 6.0 4.0 10.0 0 90 0 5.0 0 0 | gmt meca -Sa2.5c -Gblue -W1p

gmt end
