#!/bin/bash
begin GMT_App_K_5 ps
gmt coast -Rk-20/20/-20/20 -JE130.35/-0.2/3.5i -Df -Gburlywood \
	-Sazure -Wthinnest -N1/thinnest,- -B10mg2m -BWSne 
gmt end
