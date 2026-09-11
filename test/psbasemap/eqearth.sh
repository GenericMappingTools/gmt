#!/usr/bin/env bash
# Test frame, annotations, ticks and gridlines for the PROJ Equal Earth projection.
# Global with the default central meridian, global centered on the Dateline, and a subregion.
gmt begin eqearth ps
	gmt basemap -Rd -J+proj=eqearth+width=12c -Bxa60g30 -Bya30g30 -BWSen
	gmt basemap -Rg -J"+proj=eqearth +lon_0=180 +width=12c" -Bxa60g30 -Bya30g30 -BWSen -Y9c
	gmt basemap -R-100/40/-20/70 -J+proj=eqearth+width=12c -Bafg -BWSen -Y9c
gmt end show
