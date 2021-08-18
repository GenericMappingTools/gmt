#!/usr/bin/env bash
#
# Check that linear symbol scaling works in meca
# Reference https://forum.generic-mapping-tools.org/t/linear-scaling-of-moment-tensor-symbols-for-psmeca/1978/14

gmt begin seis_13
	gmt psmeca -Sm0.4+f0+l+s5e25 /dev/null -R30/40/23/30 -JM15c -B
gmt end show
