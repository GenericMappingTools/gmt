#!/usr/bin/env bash
gmt begin GMT_App_K_5
	gmt coast -R-20/20/-20/20+uk -JE130.35/-0.2/3.5i -Df -Gburlywood -Sazure -Wthinnest -N1/thinnest,- -B10mg2m -BWSne
gmt end show
