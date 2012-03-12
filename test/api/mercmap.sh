#!/bin/bash
#	$Id$
#
# Test the Mercator map maker

. functions.sh
header "Test Mercator relief image made by gmtmercmap"

ps=merc.ps

ln -fs $src/etopo2m_grd.nc .
gmtmercmap -R-30/10/0/30 -Crelief -P -W6i -S > $ps

pscmp
