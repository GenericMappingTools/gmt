#!/usr/bin/env bash
# Test pshistogram with monthly (variable) bins
ps=varbins.ps
gmt set FORMAT_TIME_PRIMARY_MAP Abbrev FORMAT_DATE_MAP "o"
gmt pshistogram -R2017-01-01T/2018-01-01T/0/1.1e6 -JX6iT/4i -Z0+w @HI_arrivals_2017.txt \
	-T2017T/2018T/1o -Gyellow -W1p -BWSne+t"Hawaii 2017 Monthly Visitor Arrivals" -P -K -Bxa1Og1o -Byaf -X1.25i > $ps
gmt set FORMAT_TIME_PRIMARY_MAP Abbrev
gmt pshistogram -R2017-01-01T/2018-01-01T/0/4e6 -JX6iT/4i -Z0+w @HI_arrivals_2017.txt \
	-T2017T/2018T/3o -Gorange -W1p -BWSne+t"Hawaii 2017 Quarterly Visitor Arrivals" -O -Bxa3Og3o -Byaf -Y5i >> $ps
