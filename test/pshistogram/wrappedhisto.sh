#!/usr/bin/env bash
# This shows the wrapped distribution from https://github.com/GenericMappingTools/gmt/issues/4992
# which happened because -N did not know about -w.  Now, we properly fit a von mises circular distro
# to this wrapped histogram.

gmt begin wrappedhisto
  gmt histogram -JX15c/10c @wrappedhisto.txt  -T0.1 -BWSen -Bxa1Of1O+l"Months" -Byaf+l"Frequency"+u"%" -Z1 -N0+p2p -wa -Glightblue -Wthinnest,black --FORMAT_TIME_PRIMARY_MAP=c -R-5/7/0/0
  gmt histogram @wrappedhisto.txt -T1 -BWSen -Bxa1Of1O+l"Months" -Byaf+l"Frequency"+u"%" -D -Z1 -N0+p2p -wa -Glightblue -Wthinnest,black --FORMAT_TIME_PRIMARY_MAP=c -Y12c -R0/12/0/0
gmt end show
