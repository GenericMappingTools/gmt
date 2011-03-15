#!/bin/bash
#
#	$Id: pscoast_JA.sh,v 1.3 2011-03-15 02:06:45 guru Exp $
# Make sure when fixed it works for all resolutions -D?

ps=pscoast_JA.ps

. ../functions.sh
header "Test pscoast for JA plot of Germany"

pscoast -JA13:25/52:31/10/7i -Rg -Gred -Sblue -Dl -P > $ps

pscmp
