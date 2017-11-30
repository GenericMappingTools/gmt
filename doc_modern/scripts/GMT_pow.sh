#!/bin/bash
#	$Id$
#
gmt psxy -R0/100/0/10 -Jx0.3ip0.5/0.15i -Bxa1p -Bya2f1 -BWSne+givory -Wthick -P -K sqrt.txt > GMT_pow.ps
gmt psxy -R -J -Sc0.075i -Ggreen -W -O sqrt10.txt >> GMT_pow.ps
