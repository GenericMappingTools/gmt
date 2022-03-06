#!/usr/bin/env bash
gmt begin GMT_latex ps
	gmt set GMT_THEME cookbook
	gmt basemap -R-200/200/0/2 -JX15c -Bxaf+l"@[\nabla^4 \psi - \Delta \sigma_{xx}^2@[ (MPa)" -BS
gmt end show
