#!/bin/bash
#	$Id$
#
# Test the Mercator map maker

header "Test Mercator relief image made by gmtmercmap"

gmtmercmap -R-30/10/0/30 -Crelief -P -W6i -S > $ps

pscmp
