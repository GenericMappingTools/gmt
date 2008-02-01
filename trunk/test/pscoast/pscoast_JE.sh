#!/bin/sh
#
#	$Id: pscoast_JE.sh,v 1.1 2008-02-01 22:23:11 guru Exp $
# Make sure when fixed it works for all resolutions -D?

ps=pscoast_JE.ps

. ../functions.sh
header "Test pscoast for JE plot of Germany"

pscoast -JE13:25/52:31/10/7i -Rg -Gred -Dl -P -B0 > $ps

pscmp
