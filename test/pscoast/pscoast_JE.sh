#!/bin/sh
#
#	$Id: pscoast_JE.sh,v 1.2 2008-04-04 18:22:28 remko Exp $
# Make sure when fixed it works for all resolutions -D?

ps=pscoast_JE.ps

. ../functions.sh
header "Test pscoast for JE plot of Germany"

pscoast -JE13:25/52:31/10/7i -Rg -Gred -Sblue -Dl -P > $ps

pscmp
