#!/bin/bash
#	$Id$
#
gmt psbasemap -R1/10000/1e20/1e25 -JX9il/6il  -Bxa2+l"Wavelength (m)" -Bya1pf3+l"Power (W)" -BWS > GMT_tut_2.ps
