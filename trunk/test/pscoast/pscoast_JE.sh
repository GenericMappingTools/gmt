#!/bin/bash
#
#	$Id: pscoast_JE.sh,v 1.3 2011-03-15 02:06:45 guru Exp $
# Make sure when fixed it works for all resolutions -D?

ps=pscoast_JE.ps

. ../functions.sh
header "Test pscoast for JE plot of Germany"

pscoast -JE13:25/52:31/10/7i -Rg -Gred -Sblue -Dl -P > $ps

pscmp
